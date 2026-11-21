#pragma once

namespace devu {

// Limita "value" ao intervalo [minValue, maxValue]:
//   - se value < minValue, devolve minValue;
//   - se value > maxValue, devolve maxValue;
//   - senão, devolve o próprio value, sem alterar nada.
//
// Isso importa pro quique na borda da tela: o objeto anda um pouquinho
// além do limite num frame, e o clamp o "puxa de volta" exatamente pra
// borda. Como devolve o valor idêntico quando está dentro do intervalo,
// comparar o resultado com o original (resultado != value) é uma forma
// exata de detectar que houve correção, ou seja, que bateu no limite.
//
// Se minValue > maxValue (ex: janela menor que o objeto), não há
// comportamento indefinido: o resultado cai em minValue ou maxValue,
// sem travar. Já std::clamp, nesse caso, seria UB.
//
// "constexpr" permite usar em tempo de compilação também, sem custo.
constexpr float clamp(float value, float minValue, float maxValue) {
  if (value < minValue) {
    return minValue;
  }
  if (value > maxValue) {
    return maxValue;
  }
  return value;
}

} // namespace devu
