// Header principal da engine (Application, clearBackground/
// drawRectangle/drawCircle, Color...) mais os utilitários de
// matemática e física: Vector2, clamp e AABB. Nada de SDL3.h nem
// Window/Renderer aqui: esses são detalhes internos da engine que o
// jogo não precisa conhecer.
#include "devu/core/Application.hpp"
#include "devu/math/Vector2.hpp"
#include "devu/math/mathUtils.hpp"
#include "devu/physics/AABB.hpp"

// Namespace anônimo: dá "linkage interno" pra TheHunterGame, ou seja,
// essa classe só existe/é visível dentro deste arquivo .cpp. Evita
// colisão de nomes se outro jogo (ou a própria engine) também tiver
// uma classe chamada TheHunterGame em outro arquivo.
namespace {

// Primeiro teste visual da engine: cenário simples (chão, parede e
// plataformas) com um quadrado parado e um círculo que anda na
// horizontal, quicando nas bordas da tela e no quadrado. Conforme a
// engine ganhar mais recursos (input, sprites, mais colisões...),
// essa classe cresce com o gameplay de verdade do The Hunter.
class TheHunterGame : public devu::Application {
public:
  // Passa a configuração fixa (título "The Hunter", janela 800x600)
  // pro construtor da classe base. O backend de renderização não é
  // especificado aqui, então usa o default RendererBackend::Auto — a
  // engine escolhe o melhor driver nativo da plataforma sozinha.
  TheHunterGame() : Application("The Hunter", 800, 600) {}

protected:
  // "override" confirma pro compilador (e pra quem lê o código) que
  // esse método está de fato substituindo um virtual da classe base —
  // se o nome/assinatura não bater com nada em Application, vira erro
  // de compilação em vez de silenciosamente criar um método novo que
  // nunca é chamado.
  void onLoad() override {
    // carregamento de assets/estado inicial do jogo entra aqui
  }

  void onUpdate(float deltaSeconds) override {
    // Integração de Euler: nova posição = posição + velocidade * dt.
    // Com a velocidade em pixels por segundo e dt em segundos, o
    // produto já sai em pixels.
    circlePosition_ += circleVelocity_ * deltaSeconds;

    // --- Colisão com os limites da tela ---
    // Limites horizontais do CENTRO do círculo: a borda esquerda do
    // círculo (centro - raio) não pode passar de 0, e a direita
    // (centro + raio) não pode passar da largura da tela. Por isso o
    // centro fica entre [raio, largura - raio]. screenWidth() é lida
    // a cada frame, então se a janela for redimensionada o limite
    // acompanha.
    const float minX = kCircleRadius;
    const float maxX = screenWidth() - kCircleRadius;

    // clamp "puxa de volta" o centro pra dentro do intervalo. Se o
    // círculo andou além do limite neste frame, clampedX vem
    // diferente de circlePosition_.x (comparação exata de floats é
    // segura: clamp devolve o valor idêntico quando está dentro).
    const float clampedX = devu::clamp(circlePosition_.x, minX, maxX);
    if (clampedX != circlePosition_.x) {
      // Bateu no limite: corrige a posição pra exatamente a borda e
      // inverte o sentido da velocidade horizontal ("retorna").
      circlePosition_.x = clampedX;
      circleVelocity_.x = -circleVelocity_.x;
    }

    // --- Colisão AABB com o quadrado branco ---
    // Monta as duas caixas deste frame. A do círculo é a caixa que o
    // envolve (centro +- raio nos dois eixos); a do quadrado vem do
    // mesmo canto/tamanho usados no desenho.
    const devu::AABB circleBox =
        devu::AABB::fromCenter(circlePosition_, kCircleRadius, kCircleRadius);
    const devu::AABB squareBox = devu::AABB::fromRect(
        kSquarePosition.x, kSquarePosition.y, kSquareSize, kSquareSize);

    if (devu::aabbIntersects(circleBox, squareBox)) {
      // Colidiu: pega o menor deslocamento que tira o círculo de
      // dentro do quadrado e aplica na posição...
      const devu::Vector2 push = devu::aabbPenetration(circleBox, squareBox);
      circlePosition_ += push;

      // ...e inverte a velocidade no eixo em que foi empurrado (se o
      // empurrão foi horizontal, só x inverte; se foi vertical, só y).
      if (push.x != 0.0f) {
        circleVelocity_.x = -circleVelocity_.x;
      }
      if (push.y != 0.0f) {
        circleVelocity_.y = -circleVelocity_.y;
      }
    }
  }

  void onDraw() override {
    // A engine já limpa a tela de preto sozinha antes de chamar
    // onDraw (ver Application::run()) — não precisa chamar nada aqui
    // pra manter o fundo preto. A linha abaixo é opcional: só existe
    // pra SUBSTITUIR esse preto padrão por um azul celeste. Comente-a
    // (ou apague) pra voltar ao preto automático da engine.
    clearBackground(135, 206, 235);

    // --- Cenário da fase (chão, parede e duas plataformas) ---
    // Desenhado ANTES do quadrado/círculo de propósito: eles ficam
    // "na frente" do cenário em vez de escondidos atrás dele — o
    // mesmo motivo pelo qual clearBackground vem antes de tudo.
    //
    // screenWidth()/screenHeight() consultam o tamanho ATUAL da
    // janela (hoje 800x600) em vez de números fixos escritos na mão —
    // se a janela for redimensionada no futuro, o cenário acompanha
    // sozinho.
    const float screenW = screenWidth();
    const float screenH = screenHeight();

    // Chão: uma faixa marrom cobrindo toda a largura da tela, colada
    // no fundo.
    constexpr float kFloorHeight = 40.0f;
    drawRectangle(0.0f, screenH - kFloorHeight, screenW, kFloorHeight,
                  devu::Color::Brown());

    // Parede: uma faixa cinza vertical na borda direita da tela, do
    // topo até o chão.
    constexpr float kWallWidth = 30.0f;
    drawRectangle(screenW - kWallWidth, 0.0f, kWallWidth,
                  screenH - kFloorHeight, devu::Color::Gray());

    // Duas plataformas flutuantes (mesmo tom de pedra da parede):
    // uma mais baixa à esquerda, outra mais alta à direita.
    drawRectangle(300.0f, 450.0f, 160.0f, 25.0f, devu::Color::Gray());
    drawRectangle(550.0f, 150.0f, 150.0f, 25.0f, devu::Color::Gray());

    // --- Quadrado e círculo ---
    // Quadrado branco (cor padrão, sem cor explícita), na posição e
    // tamanho definidos pelas constantes da classe — as mesmas usadas
    // na colisão, então o que você vê é exatamente o que colide.
    drawRectangle(kSquarePosition.x, kSquarePosition.y, kSquareSize,
                  kSquareSize);

    // Círculo vermelho, na posição atualizada por onUpdate a cada
    // frame (começa em (600, 300)).
    drawCircle(circlePosition_.x, circlePosition_.y, kCircleRadius,
               devu::Color::Red());

    // desenho de outras entidades do jogo entra aqui, DEPOIS do
    // clearBackground — senão ficariam escondidas por baixo dele.
  }

private:
  // Raio do círculo, usado pra desenhar, pro quique na tela e pra caixa
  // de colisão.
  static constexpr float kCircleRadius = 80.0f;

  // Canto superior esquerdo e lado do quadrado branco (150x150 em
  // (100, 225), como sempre foi). Ficam em constantes únicas pra
  // desenho e colisão nunca ficarem fora de sincronia.
  static constexpr devu::Vector2 kSquarePosition{100.0f, 225.0f};
  static constexpr float kSquareSize = 150.0f;

  // Posição do CENTRO do círculo, em pixels.
  devu::Vector2 circlePosition_{600.0f, 300.0f};

  // Velocidade em pixels por segundo: 200 px/s pra direita, nenhuma
  // componente vertical (movimento puramente horizontal por enquanto).
  devu::Vector2 circleVelocity_{200.0f, 0.0f};
};

} // namespace

int main(int argc, char *argv[]) {
  // Cria a instância do jogo na pilha (stack) — seu tempo de vida é só
  // o desta função main. run() só retorna quando a janela é fechada.
  TheHunterGame game;
  // O valor de retorno de run() (0 = sucesso, -1 = falha de
  // inicialização) vira o código de saída do processo.
  return game.run();
}
