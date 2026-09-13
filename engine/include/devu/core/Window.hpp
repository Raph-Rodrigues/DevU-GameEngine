#pragma once

// SDL3.h traz todos os tipos/funções da lib (SDL_Window, SDL_CreateWindow,
// SDL_Rect, SDL_DisplayID, etc.) que essa classe usa.
#include <SDL3/SDL.h>
// std::string é usado só pra receber o título da janela de forma segura
// (sem se preocupar com tamanho de buffer / lifetime de char*).
#include <string>

namespace devu {

// Encapsula a criação/destruição de uma janela SDL (RAII) e consultas
// básicas de monitor. Por enquanto é só um wrapper fino em cima da API C
// do SDL3; conforme a engine crescer (multi-janela, fullscreen, etc.)
// essa classe cresce junto.
//
// "RAII" (Resource Acquisition Is Initialization) é o motivo de existir
// construtor/destrutor aqui: a janela SDL é criada no construtor e
// destruída no destrutor, então basta um objeto Window sair de escopo
// (ou o unique_ptr que o guarda ser resetado) pra janela ser fechada
// automaticamente — sem precisar lembrar de chamar SDL_DestroyWindow
// manualmente em todo lugar.
class Window {
public:
  // width/height: tamanho inicial da janela em pixels.
  // flags: opções do SDL (redimensionável, fullscreen, etc.); o valor
  // default SDL_WINDOW_RESIZABLE é o que o main.cpp original usava.
  // O parâmetro não tem nome porque não é usado no corpo além de ser
  // repassado direto pro SDL — só o tipo e o default importam aqui.
  Window(const std::string &title, int width, int height,
         SDL_WindowFlags = SDL_WINDOW_RESIZABLE); // construtor da classe
  ~Window();                                      // descontrutor

  // Uma Window "possui" um SDL_Window* (ponteiro cru, sem contagem de
  // referência). Copiar um objeto Window copiaria o ponteiro, e aí as
  // duas cópias tentariam destruir a MESMA janela SDL no destrutor —
  // double free. Por isso cópia é proibida (deletada); só dá pra mover
  // ou usar por referência/ponteiro (é assim que Application guarda a
  // sua Window: dentro de um std::unique_ptr).
  Window(const Window &) = delete;
  Window &operator=(const Window &) = delete;

  // Dá acesso ao ponteiro SDL cru pra quem precisar chamar funções da
  // API C do SDL diretamente (ex: Renderer precisa da janela pra criar
  // o SDL_Renderer).
  SDL_Window *handle() const { return window_; }
  // true se a criação da janela deu certo (window_ != nullptr).
  bool isValid() const { return window_ != nullptr; }

  // Método estático: não depende de nenhuma instância de Window (não
  // usa window_), só consulta informação global do sistema (monitores
  // conectados). Por isso pode ser chamado como Window::primaryDisplayBounds()
  // sem precisar ter uma janela criada ainda.
  // Retorna a resolução do monitor principal, ou {0,0,0,0} se falhar.
  static SDL_Rect primaryDisplayBounds();

private:
  // Ponteiro cru pro objeto interno do SDL. Começa como nullptr e só é
  // preenchido se SDL_CreateWindow (no .cpp) tiver sucesso.
  SDL_Window *window_ = nullptr;
};

} // namespace devu
