// Único header da engine que o jogo precisa incluir diretamente — tudo
// que o jogo faz é através da API pública de Application (onLoad/
// onUpdate/onDraw/run, clearBackground/drawRectangle/drawCircle) e do
// tipo Color. Nada de SDL3.h nem Window/Renderer aqui: esses são
// detalhes internos da engine que o jogo não precisa conhecer.
#include "devu/core/Application.hpp"

// Namespace anônimo: dá "linkage interno" pra TheHunterGame, ou seja,
// essa classe só existe/é visível dentro deste arquivo .cpp. Evita
// colisão de nomes se outro jogo (ou a própria engine) também tiver
// uma classe chamada TheHunterGame em outro arquivo.
namespace {

// Primeiro teste visual da engine: um quadrado e um círculo na tela,
// sobre um fundo azul escuro. Conforme a engine ganhar mais recursos
// (input, sprites, física...), essa classe cresce com o gameplay de
// verdade do The Hunter.
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
    // pra SUBSTITUIR esse preto padrão por um azul celeste. Comente-a
    // (ou apague) pra voltar ao preto automático da engine.
    clearBackground(135, 206, 235);

    // Primeiro quadrado da engine: 150x150 pixels, canto superior
    // esquerdo em (100, 225), vermelho. drawRectangle desenha
    // qualquer retângulo — um quadrado é só o caso em que width ==
    // height.
    drawRectangle(100.0f, 225.0f, 150.0f, 150.0f);

    // Primeiro círculo da engine: centro em (600, 300), raio 80,
    // verde. Se você chamar drawCircle() sem passar cor nenhuma, o
    // default é branco (devu::Color::White()) — é a regra que você
    // pediu.
    drawCircle(600.0f, 300.0f, 80.0f, devu::Color::Red());

    // desenho de outras entidades do jogo entra aqui, DEPOIS do
    // clearBackground — senão ficariam escondidas por baixo dele.
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
