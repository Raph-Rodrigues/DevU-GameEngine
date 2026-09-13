// Único header da engine que o jogo precisa incluir diretamente — tudo
// que o jogo faz é através da API pública de Application (onLoad/
// onUpdate/onDraw/run). Nada de SDL3.h nem Window/Renderer aqui: esses
// são detalhes internos da engine que o jogo não precisa conhecer.
#include "devu/core/Application.hpp"
#include "devu/core/Color.hpp"

// Namespace anônimo: dá "linkage interno" pra TheHunterGame, ou seja,
// essa classe só existe/é visível dentro deste arquivo .cpp. Evita
// colisão de nomes se outro jogo (ou a própria engine) também tiver
// uma classe chamada TheHunterGame em outro arquivo.
namespace {

// Por enquanto o jogo não faz nada além do que a engine já faz sozinha
// (tela preta). Conforme a engine ganhar mais recursos (input, sprites,
// física...), essa classe cresce com o gameplay do The Hunter.
class TheHunterGame : public devu::Application {
public:
  // Passa a configuração fixa (título "The Hunter", janela 800x600)
  // pro construtor da classe base — é só isso que Application precisa
  // saber antes de rodar.
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
    // (void)deltaSeconds silencia o warning de "parâmetro não usado",
    // já que por enquanto não há nenhuma lógica de gameplay que
    // dependa do tempo entre frames.
    (void)deltaSeconds;
    // lógica de gameplay entra aqui
  }

  void onDraw() override {
    // A engine já limpa a tela de preto sozinha antes de chamar
    // onDraw (ver Application::run()) — não precisa chamar nada aqui
    // pra manter o fundo preto. A linha abaixo é opcional: só existe
    // pra SUBSTITUIR esse preto padrão por um azul claro. Comente-a
    // (ou apague) pra voltar ao preto automático da engine.
    clearBackground(135, 206, 250);

    // desenho de sprites/entidades do jogo entra aqui, DEPOIS do
    // clearBackground — senão ficariam escondidos por baixo dele.

    // Desenha um retângulo branco na posição (100,100) com largura 200 e altura
    // 150
    drawRectangle(100, 100, 200, 150);

    devu::Color red = {255, 99, 71};
    // Desenha um círculo branco na posição (400,300) com raio 50
    drawCircle(400, 300, 50, red);
  }
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
