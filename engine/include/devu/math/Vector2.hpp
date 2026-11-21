#pragma once

#include <cmath> // std::sqrt

namespace devu {

// Vetor 2D com componentes float: serve tanto pra posição (x, y)
// quanto pra velocidade, aceleração, direção... O significado vem do
// uso, não do tipo. É um "value type" simples: campos públicos, sem
// lógica escondida, copiado por valor (8 bytes, mais barato copiar do
// que passar por referência).
struct Vector2 {
  float x = 0.0f;
  float y = 0.0f;

  // Vetor nulo (0, 0).
  constexpr Vector2() = default;
  constexpr Vector2(float x_, float y_) : x(x_), y(y_) {}

  // --- Operações vetoriais ---
  // Soma e subtração componente a componente: (a + b) = (ax+bx, ay+by).
  constexpr Vector2 operator+(Vector2 o) const { return {x + o.x, y + o.y}; }
  constexpr Vector2 operator-(Vector2 o) const { return {x - o.x, y - o.y}; }
  // Negação: inverte o sentido do vetor, (-x, -y).
  constexpr Vector2 operator-() const { return {-x, -y}; }
  // Multiplicação/divisão por escalar: muda o módulo, mantém a direção
  // (se o escalar for positivo). É assim que "velocidade * dt" vira
  // deslocamento.
  constexpr Vector2 operator*(float s) const { return {x * s, y * s}; }
  constexpr Vector2 operator/(float s) const { return {x / s, y / s}; }

  // Versões que modificam o próprio vetor (a += b).
  constexpr Vector2 &operator+=(Vector2 o) {
    x += o.x;
    y += o.y;
    return *this;
  }
  constexpr Vector2 &operator-=(Vector2 o) {
    x -= o.x;
    y -= o.y;
    return *this;
  }
  constexpr Vector2 &operator*=(float s) {
    x *= s;
    y *= s;
    return *this;
  }
  constexpr Vector2 &operator/=(float s) {
    x /= s;
    y /= s;
    return *this;
  }

  // Igualdade EXATA entre floats (sem tolerância) — serve pra testar
  // coisas como "é o vetor nulo?", mas não pra comparar resultados de
  // contas com arredondamento.
  constexpr bool operator==(Vector2 o) const { return x == o.x && y == o.y; }
  constexpr bool operator!=(Vector2 o) const { return !(*this == o); }

  // Produto escalar: a·b = ax*bx + ay*by. Dá o cosseno do ângulo entre
  // os vetores (vezes os módulos); é 0 se forem perpendiculares.
  constexpr float dot(Vector2 o) const { return x * o.x + y * o.y; }

  // Módulo ao quadrado: x² + y². Evita a raiz quadrada, então é mais
  // barato — ideal pra comparar distâncias (se a² < b², então a < b).
  constexpr float lengthSquared() const {
    return std::pow(x, 2) + std::pow(y, 2);
  }

  // Módulo (comprimento): sqrt(x² + y²). Não é constexpr porque
  // std::sqrt só é constexpr a partir do C++26.
  float length() const { return std::sqrt(lengthSquared()); }

  // Vetor de mesma direção e módulo 1. Se o vetor for nulo (não tem
  // direção), devolve o vetor nulo em vez de dividir por zero.
  Vector2 normalized() const {
    const float len = length();
    return len > 0.0f ? Vector2{x / len, y / len} : Vector2{};
  }
};

// Permite escrever "2.0f * vetor", além de "vetor * 2.0f".
constexpr Vector2 operator*(float s, Vector2 v) { return v * s; }

} // namespace devu
