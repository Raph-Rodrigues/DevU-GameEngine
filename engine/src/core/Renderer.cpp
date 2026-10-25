#include "devu/core/Renderer.hpp"
#include "devu/core/Window.hpp"

#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>
#include <algorithm> // std::find
#include <cmath>     // std::cos, std::sin
#include <cstdio>    // std::printf
#include <numbers>   // std::numbers::pi_v
#include <string>
#include <vector>

namespace devu {

// Descobre, entre os drivers de renderização disponíveis no sistema,
// qual é o preferido nesta plataforma — e devolve o nome dele (ou
// nullptr se nenhum da lista de prioridade estiver disponível). Só é
// usada quando o backend pedido é RendererBackend::Auto.
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
  // final. O Vulkan agora entra como segunda opção em Windows/macOS
  // também: se o driver nativo da plataforma (direct3d/metal) não
  // estiver disponível por algum motivo, a engine tenta Vulkan antes
  // de cair no "deixa o SDL escolher sozinho".
#if defined(SDL_PLATFORM_WIN32)
  static const std::vector<const char *> priority = {"direct3d12", "direct3d11",
                                                     "vulkan"};
#elif defined(SDL_PLATFORM_MACOS)
  static const std::vector<const char *> priority = {"metal", "vulkan"};
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

// Traduz um backend explícito pro nome de driver que o SDL espera.
// RendererBackend::Auto devolve nullptr DE PROPÓSITO — é assim que o
// construtor sabe que deve usar pickPreferredDriver() em vez de um
// nome fixo.
const char *Renderer::backendToDriverName(RendererBackend backend) {
  switch (backend) {
  case RendererBackend::Vulkan:
    return "vulkan";
  case RendererBackend::Direct3D12:
    return "direct3d12";
  case RendererBackend::Direct3D11:
    return "direct3d11";
  case RendererBackend::Metal:
    return "metal";
  case RendererBackend::OpenGL:
    return "opengl";
  case RendererBackend::Software:
    return "software";
  case RendererBackend::Auto:
  default:
    return nullptr;
  }
}

// Construtor: recebe a Window já criada (por referência const — só
// precisa ler o handle dela, não é dono da janela) e pede ao SDL pra
// criar um renderer associado a ela. Se "backend" for Auto, usa a
// lógica antiga de prioridade por plataforma; se for qualquer outro
// valor, força exatamente aquele driver (permitindo, por exemplo,
// pedir Vulkan explicitamente mesmo numa build de Windows).
Renderer::Renderer(const Window &window, RendererBackend backend) {
  const char *driverName = (backend == RendererBackend::Auto)
                               ? pickPreferredDriver()
                               : backendToDriverName(backend);

  // Se driverName for nullptr (Auto sem nenhum preferido disponível,
  // ou nenhum outro caso já que os demais sempre retornam um nome),
  // SDL_CreateRenderer interpreta isso como "escolha automaticamente".
  renderer_ = SDL_CreateRenderer(window.handle(), driverName);
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
// renderer já criado (pode não ser o mesmo que foi pedido, se aquele
// driver falhar silenciosamente e o SDL cair pra outro).
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

void Renderer::drawRectangle(float x, float y, float width, float height,
                             Color color, bool filled) {
  setDrawColor(color.r, color.g, color.b, color.a);

  // SDL3 usa SDL_FRect (com float) pras chamadas de desenho — diferente
  // de SDL_Rect (com int), que é o tipo usado só pra bounds de monitor
  // em Window::primaryDisplayBounds().
  SDL_FRect rect{x, y, width, height};

  if (filled) {
    SDL_RenderFillRect(renderer_, &rect);
  } else {
    SDL_RenderRect(renderer_, &rect); // só o contorno
  }
}

void Renderer::drawCircle(float centerX, float centerY, float radius,
                          Color color, bool filled) {
  // Método de Arquimedes: começa de um hexágono regular (6 lados,
  // inscrito na circunferência) e DOBRA o número de lados algumas
  // vezes — 6 -> 12 -> 24 -> 48 -> 96 — até o polígono ficar visualmente
  // indistinguível de um círculo. kDoublings=4 dobrações a partir de 6
  // dá 6 << 4 = 96 lados.
  constexpr int kBaseSides = 6;
  constexpr int kDoublings = 4;
  constexpr int kSides = kBaseSides << kDoublings; // 96

  setDrawColor(color.r, color.g, color.b, color.a);

  // Calcula os kSides+1 vértices do polígono regular, igualmente
  // espaçados ao redor da circunferência (o "+1" repete o primeiro
  // ponto no final, fechando o contorno pra desenhar as linhas).
  // x = centro + raio*cos(ângulo), y = centro + raio*sen(ângulo) — a
  // parametrização usual de um círculo em coordenadas polares.
  std::vector<SDL_FPoint> points;
  points.reserve(kSides + 1);
  constexpr float kTwoPi = 2.0f * std::numbers::pi_v<float>;
  for (int i = 0; i <= kSides; ++i) {
    float angle = kTwoPi * static_cast<float>(i) / static_cast<float>(kSides);
    points.push_back(SDL_FPoint{centerX + radius * std::cos(angle),
                                centerY + radius * std::sin(angle)});
  }

  if (!filled) {
    // Contorno: só conecta os vértices em sequência.
    SDL_RenderLines(renderer_, points.data(), static_cast<int>(points.size()));
    return;
  }

  // Preenchido: o SDL não tem uma primitiva de "polígono preenchido",
  // então montamos um "leque de triângulos" (triangle fan) partindo do
  // centro — cada par de vértices vizinhos forma um triângulo com o
  // centro do círculo, e a soma de todos os triângulos preenche o
  // polígono inteiro. SDL_RenderGeometry desenha exatamente isso.
  SDL_FColor fcolor{color.r / 255.0f, color.g / 255.0f, color.b / 255.0f,
                    color.a / 255.0f};

  std::vector<SDL_Vertex> vertices;
  vertices.reserve(points.size() + 1);
  // Índice 0 do array de vértices é sempre o centro do leque.
  vertices.push_back(
      SDL_Vertex{SDL_FPoint{centerX, centerY}, fcolor, SDL_FPoint{0, 0}});
  for (const SDL_FPoint &p : points) {
    vertices.push_back(SDL_Vertex{p, fcolor, SDL_FPoint{0, 0}});
  }

  // Cada triângulo do leque usa o centro (índice 0) + dois vértices
  // consecutivos do polígono (índices i+1 e i+2, já que o índice 0 é
  // o centro e os vértices do polígono começam em 1).
  std::vector<int> indices;
  indices.reserve(static_cast<std::size_t>(kSides) * 3);
  for (int i = 0; i < kSides; ++i) {
    indices.push_back(0);
    indices.push_back(i + 1);
    indices.push_back(i + 2);
  }

  // texture = nullptr: sem textura, então a cor de cada vértice
  // (fcolor, igual em todos) é o que efetivamente pinta os triângulos.
  SDL_RenderGeometry(renderer_, nullptr, vertices.data(),
                     static_cast<int>(vertices.size()), indices.data(),
                     static_cast<int>(indices.size()));
}

} // namespace devu
