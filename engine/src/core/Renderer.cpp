#include "devu/core/Renderer.hpp"
#include "devu/core/Window.hpp"
#include "devu/core/Color.hpp"

#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>
#include <algorithm> // std::find
#include <cstdio>    // std::printf
#include <string>
#include <vector>
#include <cmath>

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

void Renderer::drawRectangle(int x, int y, int w, int h, const Color& color) {
    // Preenche um retângulo na posição (x, y) com largura w e altura h.
    // Usa SDL_FRect para precisão de ponto flutuante e SDL_RenderFillRect para preenchimento.
    setDrawColor(color.r, color.g, color.b, color.a);
    SDL_FRect rect{ static_cast<float>(x), static_cast<float>(y), static_cast<float>(w), static_cast<float>(h) };
    SDL_RenderFillRect(renderer_, &rect);
}

void Renderer::drawCircle(int x, int y, int radius, const Color& color) {
    // Desenha um círculo preenchido na posição (x, y) com o raio especificado.
    // Aproximamos o círculo usando um "triangle fan" (ventilador de triângulos) a partir do centro.
    // Isso é semelhante ao método de Arquimedes de aproximar uma circunferência com polígonos,
    // mas aqui preenchemos a área dividindo o círculo em triângulos com um vértice comum no centro.
    setDrawColor(color.r, color.g, color.b, color.a);
    const int num_segments = 32; // Número de segmentos para aproximar o círculo (quanto maior, mais suave)
    
    // Preparar os vértices para o triangle fan:
    // - Primeiro vértice é o centro do círculo
    // - Os vértices subsequentes são pontos ao longo da circunferência
    std::vector<SDL_Vertex> vertices;
    vertices.reserve(num_segments + 1); // centro + num_segments pontos da circunferência
    
    // Vértice central (cores normalizadas para 0.0-1.0)
    vertices.push_back({{ static_cast<float>(x), static_cast<float>(y) },
                        { color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f },
                        { 0.0f, 0.0f }}); // coordenadas de textura não usadas
    
    // Vértices da circunferência
    for (int i = 0; i <= num_segments; ++i) { // inclui o último ponto para fechar o círculo
        float theta = 2.0f * M_PI * static_cast<float>(i) / static_cast<float>(num_segments);
        float vx = static_cast<float>(x) + radius * cosf(theta);
        float vy = static_cast<float>(y) + radius * sinf(theta);
        vertices.push_back({{ vx, vy },
                            { color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f },
                            { 0.0f, 0.0f }});
    }
    
    // Preparar os índices para formar os triângulos do triangle fan
    // Cada triângulo é formado por: centro, ponto[i], ponto[i+1]
    std::vector<int> indices;
    indices.reserve(num_segments * 3);
    for (int i = 0; i < num_segments; ++i) {
        indices.push_back(0);           // vértice do centro
        indices.push_back(i + 1);       // ponto atual na circunferência
        indices.push_back(i + 2);       // próximo ponto na circunferência
    }
    
    // Renderizar o círculo preenchido usando geometria personalizada
    SDL_RenderGeometry(renderer_, nullptr, vertices.data(), static_cast<int>(vertices.size()),
                       indices.data(), static_cast<int>(indices.size()));
}
} // namespace devu
