#pragma once

#include "devu/core/Renderer.hpp"
#include "devu/core/Window.hpp"

#include <SDL3/SDL.h>
// unique_ptr: Window e Renderer só podem ser construídos DEPOIS de
// SDL_Init ter rodado com sucesso. Se fossem membros diretos (por
// valor), eles seriam construídos automaticamente antes do corpo do
// construtor de Application executar — cedo demais. Guardando como
// ponteiro (unique_ptr), a gente decide o momento exato de criá-los,
// dentro de init().
#include <SDL3/SDL_stdinc.h>
#include <memory>
#include <string>

namespace devu {
// Classe base da engine. Cada jogo herda de Application e sobrescreve
// onLoad/onUpdate/onDraw; o loop principal (init do SDL, criação de
// janela/renderer, poll de eventos, present, shutdown) mora aqui, não
// no jogo. Ainda sem ECS nem renderizador Vulkan próprio — isso aqui é
// só a separação inicial do que estava direto no main.cpp.
//
// Esse é o padrão "template method": a classe base define o ESQUELETO
// do loop (a ordem: eventos -> update -> clear -> draw -> present), e
// as subclasses (os jogos) só preenchem os "buracos" (onLoad/onUpdate/
// onDraw) com a lógica específica de cada jogo.
class Application {
public:
  // Só guarda a configuração (título e tamanho da janela); NÃO cria
  // nada do SDL ainda — isso só acontece dentro de run(), via init().
  Application(std::string title, int width, int height);
  // "virtual" é essencial aqui: quando um TheHunterGame (por exemplo)
  // é destruído através de um ponteiro pra Application, o destrutor
  // virtual garante que o destrutor de TheHunterGame também rode, não
  // só o de Application.
  virtual ~Application();

  // Copiar uma Application copiaria os unique_ptr<Window>/<Renderer>
  // por dentro, o que nem compila (unique_ptr não é copiável) — essas
  // linhas deixam a intenção explícita e dão um erro de compilação
  // mais claro caso alguém tente copiar um jogo por acidente.
  Application(const Application &) = delete;
  Application &operator=(const Application &) = delete;

  // Inicializa o SDL, cria janela e renderizador, roda o loop principal
  // até a janela ser fechada, e faz a limpeza no final.
  // Retorna -1 se a inicialização falhar, 0 em saída normal.

  int run();

protected:
  // "protected" porque só as subclasses (os jogos) devem chamar/
  // sobrescrever esses métodos — código de fora da hierarquia de
  // Application não deveria mexer neles diretamente.

  // Chamado uma vez, antes do primeiro frame.
  virtual void onLoad() {}
  // Chamado uma vez por frame, antes do clear/draw. deltaSeconds ainda
  // é fixo (~16ms via SDL_Delay); a assinatura já fica pronta pra
  // quando isso virar delta-time real.
  virtual void onUpdate(float deltaSeconds) { (void)deltaSeconds; }
  // Chamado uma vez por frame, antes do present. A engine já limpa a
  // tela de preto automaticamente ANTES de chamar onDraw (ver
  // Application::run()) — se o jogo não fizer nada aqui, o fundo
  // continua preto. Pra outra cor, chame clearBackground() como
  // primeira linha de onDraw e depois desenhe suas entidades por cima.
  virtual void onDraw() {}

  // Troca a cor do "pincel" do renderer e já limpa a tela inteira com
  // ela — é a forma recomendada de pintar o fundo do frame com uma
  // cor diferente do preto padrão da engine. Chame isso no início do
  // seu onDraw(), antes de desenhar qualquer entidade, senão o
  // desenho anterior fica embaixo da cor nova. Se você NUNCA chamar
  // essa função, o fundo continua preto — é o clear automático que
  // Application::run() já faz antes de onDraw.
  // r/g/b/a vão de 0 a 255 (a=255 é opaco); default a=255.
  void clearBackground(Uint8 r, Uint8 g, Uint8 b, Uint8 a = 255) {
    if (renderer_) {
      renderer_->setDrawColor(r, g, b, a);
      renderer_->clear();
    }
  }

  // Dá acesso ao SDL_Renderer* cru pra quem herdar de Application e
  // precisar desenhar algo dentro de onDraw() (ex: SDL_RenderRect,
  // SDL_RenderTexture...). Retorna nullptr se o renderer ainda não
  // existir (antes de init() rodar).
  SDL_Renderer *rendererHandle() const {
    return renderer_ ? renderer_->handle() : nullptr;
  }

private:
  // "private" porque são detalhes de implementação do ciclo de vida —
  // só run() (que é da própria classe) deveria chamar init()/shutdown()
  // diretamente.
  bool init();
  void shutdown();

  // Configuração guardada no construtor, usada só depois dentro de
  // init() pra criar a Window de fato.
  std::string title_;
  int width_;
  int height_;

  // Ver comentário do #include <memory> acima: construídos
  // manualmente dentro de init(), não no momento em que Application é
  // construída.
  std::unique_ptr<Window> window_;
  std::unique_ptr<Renderer> renderer_;
  // Guarda se init() terminou com sucesso, pra shutdown() saber se há
  // algo pra limpar (evita chamar SDL_Quit() duas vezes, por exemplo
  // se o destrutor rodar depois de run() já ter chamado shutdown()).
  bool initialized_ = false;
};
} // namespace devu
