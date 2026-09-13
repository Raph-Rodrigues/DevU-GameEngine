#include "devu/core/Window.hpp"

// cstdio: usado pra std::printf nas mensagens de erro (mesmo padrão do
// main.cpp original, que usava printf pra reportar falhas do SDL).
#include <cstdio>

namespace devu {

Window::Window(const std::string &title, int width, int height,
               SDL_WindowFlags flags) {
  // title.c_str(): SDL_CreateWindow é uma API C, então espera um
  // const char* terminado em '\0', não um std::string.
  window_ = SDL_CreateWindow(title.c_str(), width, height, flags);
  if (!window_) {
    // SDL_GetError() devolve a descrição textual do último erro
    // registrado internamente pelo SDL — é assim que a lib "levanta
    // exceção" em uma API C (não tem exceções de verdade).
    std::printf("Erro ao criar janela: %s", SDL_GetError());
  }
  // Se a criação falhar, window_ continua nullptr (valor default do
  // membro) e isValid() vai reportar false pra quem estiver checando.
}

Window::~Window() {
  // Só destrói se window_ não for nullptr: isso protege o caso em que
  // a criação falhou no construtor (não faria sentido, e seria
  // indefinido, chamar SDL_DestroyWindow(nullptr) — na prática o SDL
  // até tolera, mas o guard deixa a intenção clara).
  if (window_) {
    SDL_DestroyWindow(window_);
  }
}

SDL_Rect Window::primaryDisplayBounds() {
  int numDisplays = 0;
  // SDL_GetDisplays aloca (internamente) um array de IDs de monitor e
  // devolve o ponteiro pra esse array, preenchendo numDisplays com a
  // quantidade de elementos. Esse array é responsabilidade NOSSA
  // liberar depois com SDL_free — por isso o SDL_free(displays) no
  // final da função.
  SDL_DisplayID *displays = SDL_GetDisplays(&numDisplays);
  if (!displays || numDisplays == 0) {
    std::printf("Nenhum monitor encontrado ou falha: %s", SDL_GetError());
    // Retorno "neutro" em caso de erro: um retângulo zerado. Quem
    // chama essa função só usa o resultado pra imprimir largura/altura
    // no log, então não precisamos de um sistema de erro mais robusto
    // por enquanto.
    return SDL_Rect{0, 0, 0, 0};
  }

  // Simplificação: assume que o monitor de índice 0 é o "principal".
  // O SDL não garante isso 100% em todas as plataformas, mas funciona
  // bem na prática pra um setup com um monitor só (ou pra só pegar uma
  // resolução de referência no log).
  SDL_DisplayID mainMonitor = displays[0];
  SDL_Rect bounds{};
  if (!SDL_GetDisplayBounds(mainMonitor, &bounds)) {
    std::printf("Erro ao obter tamanho do monitor: %s", SDL_GetError());
    // bounds fica com os valores zerados de SDL_Rect{} mesmo em caso
    // de erro aqui (inicialização por chaves vazias zera os campos).
  }

  // Libera o array que o SDL alocou lá em cima — sem isso seria um
  // vazamento de memória toda vez que essa função fosse chamada.
  SDL_free(displays);
  return bounds;
}

} // namespace devu
