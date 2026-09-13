#include "devu/core/Application.hpp"

#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <utility> // std::move

namespace devu {

// std::move(title): evita copiar a string desnecessariamente — o
// chamador (ex: TheHunterGame) passa um std::string temporário
// ("The Hunter") e a gente só "rouba" o conteúdo dele pra dentro de
// title_, em vez de duplicar os caracteres.
Application::Application(std::string title, int width, int height)
    : title_(std::move(title)), width_(width), height_(height) {}

// Garante que, mesmo que run() nunca tenha sido chamado (ou tenha
// saído por uma exceção/erro), os recursos do SDL são liberados
// quando o objeto Application é destruído.
Application::~Application() { shutdown(); }

// Faz toda a inicialização "pesada": SDL, janela e renderer. Retorna
// false assim que algo falhar, já desfazendo (com SDL_Quit) o que já
// tinha sido inicializado até aquele ponto.
bool Application::init() {
  // SDL_INIT_VIDEO liga só o subsistema de vídeo (janelas, telas,
  // renderização) — não precisamos de áudio/joystick/etc. ainda. Em
  // SDL3 essa função retorna bool (true = sucesso); é diferente da
  // convenção antiga do SDL2, que retornava 0 em caso de sucesso.
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    printf("Erro ao inicializar o SDL: %s", SDL_GetError());
    return false;
  }

  // Só um log informativo com a resolução do monitor principal —
  // não é usado pra nada além disso por enquanto (ex: não redimensiona
  // a janela com base nisso ainda).
  SDL_Rect displayBounds = Window::primaryDisplayBounds();
  std::cout << "Largura: " << displayBounds.w << " pixels\n" << std::endl;
  std::cout << "Altura: " << displayBounds.h << " pixels\n" << std::endl;

  // Cria a janela de verdade agora (SDL já está inicializado).
  window_ = std::make_unique<Window>(title_, width_, height_);
  if (!window_->isValid()) {
    // A janela falhou: como SDL_Init tinha funcionado, precisamos
    // desfazer isso manualmente chamando SDL_Quit antes de sair —
    // senão o subsistema de vídeo fica "ligado" sem ninguém usar.
    SDL_Quit();
    return false;
  }

  // Cria o renderer associado à janela recém-criada.
  renderer_ = std::make_unique<Renderer>(*window_);
  if (!renderer_->isValid()) {
    // Mesma lógica: desfaz o que já tinha sido criado, na ordem
    // inversa de criação (renderer já falhou/é nulo, então só reseta
    // a window e chama SDL_Quit).
    window_.reset();
    SDL_Quit();
    return false;
  }

  // Log de debug: confirma qual driver de renderização o SDL
  // realmente escolheu (útil pra conferir se pickPreferredDriver()
  // conseguiu o que queria).
  std::cout << "========================================" << std::endl;
  std::cout << " O driver de renderizacao ativo e: "
            << renderer_->activeDriveName() << std::endl;
  std::cout << "========================================" << std::endl;

  // Só marca como inicializado se TUDO acima deu certo — é essa flag
  // que shutdown() usa pra saber se precisa limpar algo.
  initialized_ = true;
  return true;
}

// Libera, na ordem inversa de criação, tudo que init() criou. Só faz
// alguma coisa se initialized_ for true (ou seja, se init() realmente
// tiver chegado ao fim com sucesso).
void Application::shutdown() {
  if (!initialized_) {
    return;
  }
  // reset() chama o destrutor do objeto apontado (~Renderer/~Window)
  // e zera o unique_ptr. Ordem importa: o renderer depende da janela
  // existir, então ele é destruído primeiro, a janela por último —
  // exatamente ao contrário da ordem em que init() os criou.
  renderer_.reset();
  window_.reset();
  // Desliga todos os subsistemas do SDL que foram inicializados
  // (nesse caso, só o de vídeo).
  SDL_Quit();
  initialized_ = false;
}

// O loop principal da aplicação/jogo.
int Application::run() {
  if (!init()) {
    return -1;
  }

  // Hook pra o jogo carregar assets/estado inicial, depois que a
  // janela e o renderer já existem.
  onLoad();

  bool running = true;
  SDL_Event event;

  while (running) {
    // Esvazia TODOS os eventos pendentes da fila do SDL neste frame
    // (teclado, mouse, fechar janela, etc.) — mesmo que a gente só
    // trate um tipo de evento por enquanto.
    while (SDL_PollEvent(&event)) {
      // SDL_EVENT_QUIT é disparado quando o usuário clica no "X" da
      // janela (ou manda um sinal equivalente do SO).
      if (event.type == SDL_EVENT_QUIT) {
        running = false;
      }
    }

    // Delta-time fixo (~16ms, aproximando 60 FPS) por enquanto — a
    // engine ainda não mede o tempo real entre frames (isso viria de
    // SDL_GetTicks() ou similar, guardando o timestamp do frame
    // anterior).
    onUpdate(0.016f);

    // Limpa a tela inteira com uma cor sólida antes de desenhar o
    // frame atual. Sem isso, o desenho do frame anterior "vazaria"
    // pro próximo.
    SDL_SetRenderDrawColor(rendererHandle(), 0, 0, 0, 255); // preto opaco
    SDL_RenderClear(rendererHandle());

    // Hook pra o jogo desenhar suas próprias entidades/sprites, entre
    // o clear e o present.
    onDraw();

    // "Apresenta" o frame: troca o buffer que acabamos de desenhar
    // pelo que está sendo mostrado na tela (double buffering).
    SDL_RenderPresent(rendererHandle());
    // Limitador de frame rate bem simples: só espera 16ms fixos, não
    // é um vsync/timing preciso, mas evita que o loop rode a 100% de
    // CPU sem necessidade.
    SDL_Delay(16);
  }

  // Libera janela/renderer/SDL assim que o loop termina (janela
  // fechada), antes mesmo do objeto Application ser destruído — o
  // destrutor chama shutdown() de novo depois, mas como initialized_
  // já é false nesse ponto, essa segunda chamada não faz nada.
  shutdown();
  return 0;
}

} // namespace devu
