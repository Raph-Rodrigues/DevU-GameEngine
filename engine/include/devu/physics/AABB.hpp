#pragma once

#include "devu/math/Vector2.hpp"

namespace devu {

// AABB = "Axis-Aligned Bounding Box": uma caixa retangular cujos lados
// são sempre paralelos aos eixos x e y (nunca rotacionada). Por ser tão
// simples, testar se duas caixas se sobrepõem custa só 4 comparações —
// por isso é a base de quase todo sistema de colisão 2D.
//
// Usamos o sistema de coordenadas da tela (y cresce PRA BAIXO), então o
// canto "mínimo" é o superior esquerdo e o "máximo" é o inferior direito.
struct AABB {
  Vector2 topLeft;     // (xMin, yMin)
  Vector2 bottomRight; // (xMax, yMax)

  // Cria a caixa a partir do canto superior esquerdo + largura/altura —
  // o mesmo formato que drawRectangle(x, y, width, height) usa.
  static constexpr AABB fromRect(float x, float y, float width, float height) {
    return AABB{Vector2{x, y}, Vector2{x + width, y + height}};
  }

  // Cria a caixa a partir do CENTRO e das meias-dimensões (distância do
  // centro até cada lado). É o jeito natural pra um círculo: a caixa que
  // o envolve tem halfWidth = halfHeight = raio.
  static constexpr AABB fromCenter(Vector2 center, float halfWidth,
                                   float halfHeight) {
    return AABB{Vector2{center.x - halfWidth, center.y - halfHeight},
                Vector2{center.x + halfWidth, center.y + halfHeight}};
  }

  constexpr Vector2 center() const {
    return Vector2{(topLeft.x + bottomRight.x) * 0.5f,
                   (topLeft.y + bottomRight.y) * 0.5f};
  }
};

// Teste de colisão AABB: duas caixas se sobrepõem se, e somente se, seus
// intervalos se sobrepõem NOS DOIS EIXOS ao mesmo tempo. Num eixo, os
// intervalos [a.min, a.max] e [b.min, b.max] se sobrepõem quando
// a.min < b.max E a.max > b.min. Se houver um eixo "separador" (um vão
// entre as caixas), não há colisão.
//
// Os "<" e ">" são estritos de propósito: caixas que só se TOCAM na borda
// (sem entrar uma na outra) não contam como colisão. Isso evita que o
// objeto fique "grudado" depois de ser empurrado pra fora.
constexpr bool aabbIntersects(const AABB &a, const AABB &b) {
  return a.topLeft.x < b.bottomRight.x && a.bottomRight.x > b.topLeft.x &&
         a.topLeft.y < b.bottomRight.y && a.bottomRight.y > b.topLeft.y;
}

// Resolução da colisão: devolve o MENOR deslocamento que, somado à
// posição de "a", tira "a" de dentro de "b" (o "minimum translation
// vector"). Devolve (0, 0) se não houver colisão.
//
// Como funciona: em cada eixo, mede-se o quanto as caixas se
// interpenetram (overlap). Empurrar pelo eixo de MENOR penetração é o
// caminho mais curto pra separá-las. O sinal do empurrão depende de qual
// lado "a" está em relação a "b" (comparando os centros).
constexpr Vector2 aabbPenetration(const AABB &a, const AABB &b) {
  if (!aabbIntersects(a, b)) {
    return Vector2{};
  }

  // Interpenetração em x: da maior borda esquerda até a menor borda
  // direita (o trecho em comum dos dois intervalos). Idem em y.
  const float overlapX =
      (a.bottomRight.x < b.bottomRight.x ? a.bottomRight.x : b.bottomRight.x) -
      (a.topLeft.x > b.topLeft.x ? a.topLeft.x : b.topLeft.x);
  const float overlapY =
      (a.bottomRight.y < b.bottomRight.y ? a.bottomRight.y : b.bottomRight.y) -
      (a.topLeft.y > b.topLeft.y ? a.topLeft.y : b.topLeft.y);

  const Vector2 ca = a.center();
  const Vector2 cb = b.center();

  if (overlapX < overlapY) {
    // Separar na horizontal: se "a" está à esquerda de "b", empurra pra
    // esquerda (negativo); senão, pra direita.
    return Vector2{ca.x < cb.x ? -overlapX : overlapX, 0.0f};
  }
  // Separar na vertical (em empate, também cai aqui): y cresce pra
  // baixo, então se "a" está acima de "b" o empurrão é negativo (sobe).
  return Vector2{0.0f, ca.y < cb.y ? -overlapY : overlapY};
}

} // namespace devu
