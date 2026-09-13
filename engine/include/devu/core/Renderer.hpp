#pragma once

// Incluído com caminho local (mesma pasta) em vez de "devu/core/Window.hpp".
// Funciona porque includes entre aspas procuram primeiro no diretório do
// próprio arquivo que está incluindo — como Renderer.hpp mora na mesma
// pasta que Window.hpp (engine/include/devu/core/), o compilador acha.
#include "Window.hpp"
#include "Color.hpp"
#include <SDL3/SDL.h>
#include <SDL3/SDL_stdinc.h>
#include <string>

namespace devu {
// Declaração adiantada de Window. Como o Window.hpp completo já foi
// incluído logo acima, essa linha é redundante aqui (o compilador já
// conhece a classe inteira) — mas não causa erro nem é maliciosa, só
// deixou de ser necessária.
class Window;

// Encapsula a criação/destruição do SDL_Renderer (RAII, mesmo espírito
// da classe Window) e a escolha de qual driver de renderização usar em
// cada plataforma.
class Renderer {
public:
  // "explicit" impede que um Window seja convertido implicitamente pra
  // Renderer em situações como `Renderer r = minhaWindow;` (que criaria
  // um Renderer novo sem o programador perceber). Força a escrita
  // explícita `Renderer r(minhaWindow);`.
  explicit Renderer(const Window &window);
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
  // preferência de pickPreferredDriver() foi respeitada.
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

  // Desenha um retângulo contornado na posição (x, y) com largura w e altura h.
  // A cor padrão é branca.
  void drawRectangle(int x, int y, int w, int h, const Color& color = Color());

  // Desenha um círculo contornado na posição (x, y) com o raio especificado.
  // A cor padrão é branca. Usa aproximação de polígono (Algoritmo de Arquimedes).
  void drawCircle(int x, int y, int radius, const Color& color = Color());

private:
  // Função auxiliar interna (não faz parte da API pública da classe):
  // decide qual driver pedir ao SDL_CreateRenderer com base na
  // plataforma em que o código foi compilado. É "static" porque não
  // precisa de nenhum SDL_Renderer já existente pra decidir isso.
  static const char *pickPreferredDriver();

  SDL_Renderer *renderer_ = nullptr;
};

} // namespace devu
