#include "devu/core/Renderer.hpp"
#include "devu/core/Window.hpp"

#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>
#include <algorithm> // std::find
#include <cstdio>    // std::printf
#include <string>
#include <vector>

namespace devu {

// Descobre, entre os drivers de renderização disponíveis no sistema,
// qual é o preferido nesta plataforma — e devolve o nome dele (ou
// nullptr se nenhum da lista de prioridade estiver disponível).
const char *Renderer::pickPreferredDriver() {
  // SDL_GetNumRenderDrivers()/SDL_GetRenderDriver(i): API do SDL pra
  // enumerar os backends de renderização compilados na lib (pode ter
  // vulkan, opengl, direct3d11, software, etc. dependendo de como o
  // SDL foi buildado). Aqui a gente só lista os nomes num vector pra
  // poder pesquisar depois com std::find.
  int numDrivers = SDL_GetNumRenderDrivers();
  std::vector<std::string> available;
  for (int i = 0; i < numDrivers; i++) {
    available.emplace_back(SDL_GetRenderDriver(i));
  }

  // Lista de prioridade por plataforma, decidida em tempo de
  // COMPILAÇÃO via macros de pré-processador (SDL_PLATFORM_*, que o
  // próprio SDL3 define). Ou seja: um binário compilado no Linux só
  // carrega a lista {"vulkan"} — as outras nem existem no executável
  // final.
#if defined(SDL_PLATFORM_WIN32)
  static const std::vector<const char *> priority = {"direct3d12",
                                                     "direct3d11"};
#elif defined(SDL_PLATFORM_MACOS)
  static const std::vector<const char *> priority = {"metal"};
#elif defined(SDL_PLATFORM_LINUX)
  static const std::vector<const char *> priority = {"vulkan"};
#else
  // Qualquer outra plataforma: lista vazia, cai direto no "return
  // nullptr" do final e deixa o SDL decidir sozinho.
  static const std::vector<const char *> priority = {};
#endif

  // Percorre a lista de prioridade NA ORDEM e devolve o primeiro nome
  // que também aparecer na lista "available" (ou seja: o mais
  // preferido dentre os que realmente existem nesta máquina).
  for (const char *name : priority) {
    if (std::find(available.begin(), available.end(), name) !=
        available.end()) {
      return name;
    }
  }
  return nullptr; // nenhum preferido escolhido deixa o sdl escolher
}

// Construtor: recebe a Window já criada (por referência const — só
// precisa ler o handle dela, não é dono da janela) e pede ao SDL pra
// criar um renderer associado a ela, usando o driver escolhido acima.
Renderer::Renderer(const Window &window) {
  // Se pickPreferredDriver() retornar nullptr, SDL_CreateRenderer
  // interpreta isso como "escolha automaticamente" — é o mesmo
  // comportamento de passar nullptr direto, só que agora com uma
  // tentativa de preferência antes do fallback.
  renderer_ = SDL_CreateRenderer(window.handle(), pickPreferredDriver());
  if (!renderer_) {
    std::printf("Erro ao criar renderizador: %s", SDL_GetError());
  }
}

Renderer::~Renderer() {
  // Guard igual ao de Window: só destrói se a criação tiver
  // funcionado.
  if (renderer_) {
    SDL_DestroyRenderer(renderer_);
  }
}

// Consulta o SDL pelo nome do driver que ficou ativo de fato no
// renderer já criado (pode não ser o mesmo que pickPreferredDriver()
// pediu, se aquele driver falhar silenciosamente e o SDL cair pra
// outro).
std::string Renderer::activeDriveName() const {
  if (!renderer_) {
    return "Desconhecido!";
  }

  // SDL3 usa um sistema de "propriedades" (SDL_PropertiesID) genérico
  // pra expor metadados de vários objetos internos. Aqui a gente pega
  // a tabela de propriedades do renderer e lê a propriedade de string
  // "nome do driver" dela, com "Desconhecido" como valor default caso
  // a propriedade não exista por algum motivo.
  SDL_PropertiesID props = SDL_GetRendererProperties(renderer_);
  return SDL_GetStringProperty(props, SDL_PROP_RENDERER_NAME_STRING,
                               "Desconhecido");
}

// Só repassa os quatro valores pro SDL — é a mesma chamada que antes
// estava espalhada direto dentro de Application::run() (e, antes
// disso, direto no main.cpp original).

void Renderer::setDrawColor(Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
  SDL_SetRenderDrawColor(renderer_, r, g, b, a);
}

// Preenche toda a área de desenho com a cor definida pela última
// chamada a setDrawColor(). Se ninguém tiver chamado setDrawColor()
// ainda, usa a cor default do próprio SDL, que é BRANCO opaco (não
// preto!) — por isso Application::run() sempre chama
// clearBackground(0, 0, 0, 255) automaticamente antes de onDraw: sem
// isso, o "preto padrão da engine" na verdade apareceria branco.

void Renderer::clear() { SDL_RenderClear(renderer_); }
} // namespace devu
