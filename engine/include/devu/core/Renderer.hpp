#pragma once

// Incluído com caminho local (mesma pasta) em vez de "devu/core/Window.hpp".
// Funciona porque includes entre aspas procuram primeiro no diretório do
// próprio arquivo que está incluindo — como Renderer.hpp mora na mesma
// pasta que Window.hpp (engine/include/devu/core/), o compilador acha.
#include "Window.hpp"
#include "devu/core/Color.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_stdinc.h>
#include <string>

namespace devu {
// Declaração adiantada de Window. Como o Window.hpp completo já foi
// incluído logo acima, essa linha é redundante aqui (o compilador já
// conhece a classe inteira) — mas não causa erro nem é maliciosa, só
// deixou de ser necessária.
class Window;

// Backend de renderização que o SDL deve usar por baixo dos panos.
// "Auto" (o default) mantém a lógica antiga: pickPreferredDriver()
// escolhe o melhor driver NATIVO de cada plataforma (direct3d no
// Windows, metal no macOS, vulkan no Linux), com fallback pra vulkan
// nas outras duas caso o nativo não esteja disponível. Os outros
// valores forçam um driver específico, ignorando a plataforma — é
// isso que permite, no futuro, deixar o jogo (ou até o usuário final)
// escolher explicitamente "quero rodar em Vulkan" em vez de aceitar a
// escolha automática.
enum class RendererBackend {
  Auto,
  Vulkan,
  Direct3D12,
  Direct3D11,
  Metal,
  OpenGL,
  Software,
};

// Encapsula a criação/destruição do SDL_Renderer (RAII, mesmo espírito
// da classe Window), a escolha de qual driver de renderização usar em
// cada plataforma, e as operações de desenho básicas (limpar tela,
// retângulo, círculo).
class Renderer {
public:
  // "explicit" impede que um Window seja convertido implicitamente pra
  // Renderer em situações como `Renderer r = minhaWindow;` (que criaria
  // um Renderer novo sem o programador perceber). Força a escrita
  // explícita `Renderer r(minhaWindow);`.
  // backend: RendererBackend::Auto (default) usa a lógica de
  // "melhor driver da plataforma"; qualquer outro valor força
  // exatamente aquele driver (útil pra testar/forçar Vulkan mesmo no
  // Windows, por exemplo).
  explicit Renderer(const Window &window,
                    RendererBackend backend = RendererBackend::Auto);
  ~Renderer();

  // Mesmo motivo do Window: copiar duplicaria o ponteiro SDL_Renderer*
  // e os dois destrutores tentariam destruir o mesmo renderer.
  Renderer(const Renderer &) = delete;
  Renderer &operator=(const Renderer &) = delete;

  // Ponteiro cru pro SDL_Renderer, usado pelas chamadas de desenho
  // (SDL_RenderClear, SDL_RenderPresent, etc.) que ficam em Application.
  SDL_Renderer *handle() const { return renderer_; }
  bool isValid() const { return renderer_ != nullptr; }

  // Nome do driver que o SDL realmente escolheu no final (ex: "vulkan",
  // "direct3d11", "opengl"...). Útil pra log/debug, pra confirmar que a
  // preferência de pickPreferredDriver()/backendToDriverName() foi
  // respeitada.
  std::string activeDriveName() const;

  // Troca a cor do "pincel" atual do renderer (a cor usada pelas
  // próximas chamadas de desenho, incluindo clear()). r/g/b/a vão de
  // 0 a 255; a=255 é totalmente opaco. Uint8 é o próprio typedef do
  // SDL pra "byte sem sinal", então bate exatamente com o que
  // SDL_SetRenderDrawColor espera.

  void setDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255);

  // Preenche a tela inteira com a cor do pincel atual (a última que
  // foi passada pra setDrawColor). Precisa ser chamado UMA VEZ por
  // frame, antes de desenhar qualquer outra coisa — senão o desenho
  // do frame anterior continua visível por baixo.
  void clear();

  // Desenha um retângulo alinhado aos eixos, com o canto superior
  // esquerdo em (x, y) e tamanho (width, height) — coordenadas em
  // pixels, com origem (0,0) no canto superior esquerdo da janela
  // (padrão do SDL). "color" default é branco, se nada for passado.
  // filled=true (default) preenche a área toda; filled=false desenha
  // só o contorno.
  void drawRectangle(float x, float y, float width, float height,
                     Color color = Color::White(), bool filled = true);

  // Desenha um círculo com centro em (centerX, centerY) e raio
  // "radius". "color" default é branco. filled=true (default) preenche
  // o círculo; filled=false desenha só o contorno.
  //
  // Como o SDL não tem uma primitiva de "círculo" pronta, a gente
  // aproxima a circunferência por um polígono regular com muitos
  // lados — a mesma ideia do método de Arquimedes pra calcular π:
  // começar de um hexágono (6 lados) e ir DOBRANDO o número de lados
  // (12, 24, 48, 96...), cada dobração chegando mais perto do círculo
  // "de verdade". Com 96 lados a diferença já é imperceptível a olho
  // nu nas resoluções normais de tela. Ver Renderer.cpp pra detalhes
  // da implementação.
  void drawCircle(float centerX, float centerY, float radius,
                  Color color = Color::White(), bool filled = true);

private:
  // Função auxiliar interna (não faz parte da API pública da classe):
  // decide qual driver pedir ao SDL_CreateRenderer com base na
  // plataforma em que o código foi compilado, quando backend == Auto.
  // É "static" porque não precisa de nenhum SDL_Renderer já existente
  // pra decidir isso.
  static const char *pickPreferredDriver();

  // Traduz um RendererBackend explícito (Vulkan, Metal, etc.) pro
  // nome de driver que o SDL_CreateRenderer espera como string.
  // RendererBackend::Auto devolve nullptr de propósito — é o sinal
  // pra Renderer::Renderer() cair no pickPreferredDriver() antigo em
  // vez de forçar um nome fixo.
  static const char *backendToDriverName(RendererBackend backend);

  SDL_Renderer *renderer_ = nullptr;
};

} // namespace devu
