#pragma once

// Uint8 é o typedef do SDL pra "byte sem sinal" (0-255) — o mesmo tipo
// que SDL_SetRenderDrawColor espera pra r/g/b/a.
#include <SDL3/SDL_stdinc.h>

namespace devu {

// Cor RGBA simples usada por todas as funções de desenho da engine
// (clearBackground, drawRectangle, drawCircle...). r/g/b/a vão de 0 a
// 255; a=255 é totalmente opaco, a=0 é totalmente transparente.
//
// É um "value type" comum (igual SDL_Color, mas com construtor default
// já apontando pro branco): os campos são públicos de propósito, não
// tem lógica nenhuma além de guardar 4 números.
struct Color {
  Uint8 r = 255;
  Uint8 g = 255;
  Uint8 b = 255;
  Uint8 a = 255;

  // Construtor default: branco opaco. É esse construtor que roda
  // quando uma função de desenho usa "Color::White()" (ou um Color{})
  // como valor padrão de parâmetro, satisfazendo a regra de "se
  // nenhuma cor for passada, usa branco".
  constexpr Color() = default;

  // Construtor explícito com os 4 componentes; "a" tem default 255
  // (opaco) pra não precisar escrever sempre os 4 valores.
  constexpr Color(Uint8 r_, Uint8 g_, Uint8 b_, Uint8 a_ = 255)
      : r(r_), g(g_), b(b_), a(a_) {}

  // Cores prontas, só por conveniência (evita ficar escrevendo
  // Color(255, 0, 0) toda vez). "constexpr" + "static" significa que
  // são calculadas em tempo de compilação, sem nenhum custo em
  // runtime.
  static constexpr Color White() { return Color{255, 255, 255, 255}; }
  static constexpr Color Black() { return Color{0, 0, 0, 255}; }
  static constexpr Color Red() { return Color{255, 0, 0, 255}; }
  static constexpr Color Green() { return Color{0, 255, 0, 255}; }
  static constexpr Color Blue() { return Color{0, 0, 255, 255}; }
};

} // namespace devu
