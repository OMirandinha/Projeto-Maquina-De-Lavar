
#include <xc.h>

// Frequencia do clock: cristal externo de 20 MHz
#define _XTAL_FREQ 20000000UL

// Configuracoes de funcionamento do PIC16F877A
#pragma config FOSC = HS      // Oscilador externo de alta frequencia
#pragma config WDTE = OFF     // Watchdog Timer desativado
#pragma config PWRTE = ON     // Temporizador de inicializacao ativado
#pragma config BOREN = ON     // Brown-out Reset ativado
#pragma config LVP = OFF      // Programacao em baixa tensao desativada
#pragma config CPD = OFF      // Protecao da memoria EEPROM desativada
#pragma config WRT = OFF      // Protecao de escrita desativada
#pragma config CP = OFF       // Protecao do codigo desativada

// ---------------- LCD ----------------
// LCD compativel com HD44780 em modo de 4 bits.
// RS seleciona comando ou dado; EN habilita a transferencia.
// Os dados sao enviados pelos pinos RD4 a RD7.

#define LCD_RS PORTDbits.RD0
#define LCD_EN PORTDbits.RD1

// Botoes conectados a RC0, RC1 e RC2.
// Considera botoes ativos em nivel baixo:
// pressionado = 0; solto = 1.
#define BTN_PROXIMO    PORTCbits.RC0
#define BTN_SELECIONAR PORTCbits.RC1
#define BTN_VOLTAR     PORTCbits.RC2

// Estados que identificam qual tela esta ativa.
typedef enum {
    MENU_PRINCIPAL,
    MENU_PROGRAMAS,
    MENU_CONFIGURACOES,
    MENU_SOBRE,
    MENU_SAIR
} EstadoMenu;

// Programas disponiveis para selecao.
// A lavagem pesada possui ciclo funcional; os demais mantem o comportamento original.
typedef enum {
    RAPIDA,
    NORMAL,
    PESADA,
    ENXAGUE_CENTRIFUGACAO,
    NENHUM_PROGRAMA
} Programa;

// Estado atual da interface e programa selecionado.
EstadoMenu estado = MENU_PRINCIPAL;
Programa programaSelecionado = NENHUM_PROGRAMA;

// Indice da opcao atualmente selecionada na tela.
unsigned char opcao = 0;

// Controla a navegacao apos selecionar "Sair".
unsigned char sair = 0;


// ==================================================
// FUNCOES DE COMUNICACAO COM O LCD
// ==================================================

// Gera um pulso no pino EN para o LCD receber os dados.
void lcdPulso(void)
{
    LCD_EN = 1;
    __delay_us(2);
    LCD_EN = 0;
    __delay_us(50);
}

// Envia os quatro bits mais significativos de um valor.
// O LCD recebe primeiro a metade superior do byte.
void lcdNibble(unsigned char valor)
{
    PORTD = (PORTD & 0x0F) | (valor & 0xF0);
    lcdPulso();
}

// Envia um comando ao LCD, como limpar a tela ou
// posicionar o cursor.
void lcdComando(unsigned char comando)
{
    LCD_RS = 0; // RS = 0 indica comando

    lcdNibble(comando);
    lcdNibble(comando << 4);

    // Limpar e retornar ao inicio exigem mais tempo.
    if (comando == 0x01 || comando == 0x02)
        __delay_ms(2);
}

// Envia um caractere para ser exibido no LCD.
void lcdCaractere(char c)
{
    LCD_RS = 1; // RS = 1 indica dado/caractere

    lcdNibble(c);
    lcdNibble(c << 4);
}

// Exibe uma string caractere por caractere.
void lcdTexto(const char *texto)
{
    while (*texto != '\0')
        lcdCaractere(*texto++);
}

// Posiciona o cursor na linha e coluna informadas.
// O LCD 16x2 utiliza enderecos diferentes por linha.
void lcdPosicao(unsigned char linha, unsigned char coluna)
{
    unsigned char endereco;

    endereco = (linha == 0 ? 0x80 : 0xC0) + coluna;
    lcdComando(endereco);
}

// Apaga o conteudo do LCD.
void lcdLimpar(void)
{
    lcdComando(0x01);
}

// Inicializa o LCD no modo de comunicacao de 4 bits.
void lcdInicializar(void)
{
    __delay_ms(20);

    LCD_RS = 0;
    LCD_EN = 0;

    // Sequencia inicial recomendada para o controlador HD44780.
    lcdNibble(0x30);
    __delay_ms(5);

    lcdNibble(0x30);
    __delay_us(150);

    lcdNibble(0x30);
    lcdNibble(0x20);

    lcdComando(0x28); // Interface de 4 bits, duas linhas
    lcdComando(0x0C); // Display ligado, cursor desligado
    lcdComando(0x06); // Cursor avanca apos cada caractere
    lcdLimpar();
}


// ==================================================
// FUNCOES DE EXIBICAO DOS MENUS
// ==================================================

// Exibe duas linhas de texto no LCD.
void mostrarTela(const char *linha1, const char *linha2)
{
    lcdLimpar();

    lcdPosicao(0, 0);
    lcdTexto(linha1);

    lcdPosicao(1, 0);
    lcdTexto(linha2);
}

// Exibe o menu principal.
// Cada valor de "opcao" corresponde a uma posicao.
void mostrarMenuPrincipal(void)
{
    switch (opcao) {
        case 0:
            mostrarTela("> Programas", "  Configuracoes");
            break;

        case 1:
            mostrarTela("  Programas", "> Configuracoes");
            break;

        case 2:
            mostrarTela("> Sobre", "  Sair");
            break;

        case 3:
            mostrarTela("  Sobre", "> Sair");
            break;
    }
}

// Exibe os programas disponiveis.
// O simbolo ">" indica o item selecionado.
void mostrarMenuProgramas(void)
{
    switch (opcao) {
        case 0:
            mostrarTela("> Lavagem rapida", "  Lavagem normal");
            break;

        case 1:
            mostrarTela("  Lavagem rapida", "> Lavagem normal");
            break;

        case 2:
            mostrarTela("> Lavagem pesada", "  Enxag+Centrif.");
            break;

        case 3:
            mostrarTela("  Lavagem pesada", "> Enxag+Centrif.");
            break;
    }
}

// Tela provisoria para configuracoes futuras.
void mostrarConfiguracoes(void)
{
    mostrarTela("Configuracoes", "Voltar: botao VOL");
}

// Apresenta informacoes basicas do projeto.
void mostrarSobre(void)
{
    mostrarTela("Simulador lavar", "PIC16F877A / C");
}


// ==================================================
// FUNCOES DE LAVAGEM
// ==================================================

// Executa um ciclo de lavagem rapida.
// RB0 = entrada de agua; RB1 = motor de lavagem;
// RB2 = bomba de drenagem; RB3 = acionamento da centrifugacao.
// Os tempos abaixo sao valores demonstrativos em segundos.
void lavagemRapida(void)
{
    unsigned char tempo;

    // Configura RB0 a RB3 como saidas e inicia tudo desligado.
    TRISB = 0xF0;
    PORTB = 0x00;

    // Etapa 1: entrada de agua.
    mostrarTela("Enchendo agua", "Lavagem rapida");
    PORTBbits.RB0 = 1;
    for (tempo = 0; tempo < 5; tempo++)
        __delay_ms(1000);
    PORTBbits.RB0 = 0;

    // Etapa 2: agitacao da roupa.
    mostrarTela("Lavando roupa", "Lavagem rapida");
    PORTBbits.RB1 = 1;
    for (tempo = 0; tempo < 10; tempo++)
        __delay_ms(1000);
    PORTBbits.RB1 = 0;

    // Etapa 3: retirada da agua.
    mostrarTela("Drenando agua", "Lavagem rapida");
    PORTBbits.RB2 = 1;
    for (tempo = 0; tempo < 5; tempo++)
        __delay_ms(1000);
    PORTBbits.RB2 = 0;

    // Etapa 4: entrada de agua para o enxague.
    mostrarTela("Enchendo agua", "Enxague rapido");
    PORTBbits.RB0 = 1;
    for (tempo = 0; tempo < 5; tempo++)
        __delay_ms(1000);
    PORTBbits.RB0 = 0;

    // Etapa 5: movimentacao da roupa durante o enxague.
    mostrarTela("Enxaguando", "Lavagem rapida");
    PORTBbits.RB1 = 1;
    for (tempo = 0; tempo < 5; tempo++)
        __delay_ms(1000);
    PORTBbits.RB1 = 0;

    // Etapa 6: drenagem e centrifugacao.
    mostrarTela("Centrifugando", "Lavagem rapida");
    PORTBbits.RB2 = 1;
    PORTBbits.RB3 = 1;
    for (tempo = 0; tempo < 10; tempo++)
        __delay_ms(1000);
    PORTB = 0x00;

    mostrarTela("Ciclo concluido", "Lavagem rapida");
    __delay_ms(2000);
}

// Executa um ciclo de lavagem normal, com etapas mais longas.
void lavagemNormal(void)
{
    unsigned char tempo;

    // Configura RB0 a RB3 como saidas e inicia tudo desligado.
    TRISB = 0xF0;
    PORTB = 0x00;

    // Etapa 1: entrada de agua.
    mostrarTela("Enchendo agua", "Lavagem normal");
    PORTBbits.RB0 = 1;
    for (tempo = 0; tempo < 10; tempo++)
        __delay_ms(1000);
    PORTBbits.RB0 = 0;

    // Etapa 2: agitacao da roupa.
    mostrarTela("Lavando roupa", "Lavagem normal");
    PORTBbits.RB1 = 1;
    for (tempo = 0; tempo < 25; tempo++)
        __delay_ms(1000);
    PORTBbits.RB1 = 0;

    // Etapa 3: retirada da agua.
    mostrarTela("Drenando agua", "Lavagem normal");
    PORTBbits.RB2 = 1;
    for (tempo = 0; tempo < 8; tempo++)
        __delay_ms(1000);
    PORTBbits.RB2 = 0;

    // Etapa 4: entrada de agua para o enxague.
    mostrarTela("Enchendo agua", "Enxague normal");
    PORTBbits.RB0 = 1;
    for (tempo = 0; tempo < 8; tempo++)
        __delay_ms(1000);
    PORTBbits.RB0 = 0;

    // Etapa 5: movimentacao da roupa durante o enxague.
    mostrarTela("Enxaguando", "Lavagem normal");
    PORTBbits.RB1 = 1;
    for (tempo = 0; tempo < 12; tempo++)
        __delay_ms(1000);
    PORTBbits.RB1 = 0;

    // Etapa 6: drenagem e centrifugacao.
    mostrarTela("Centrifugando", "Lavagem normal");
    PORTBbits.RB2 = 1;
    PORTBbits.RB3 = 1;
    for (tempo = 0; tempo < 15; tempo++)
        __delay_ms(1000);
    PORTB = 0x00;

    mostrarTela("Ciclo concluido", "Lavagem normal");
    __delay_ms(2000);
}


// ==================================================
// CICLO DE LAVAGEM PESADA
// ==================================================

// Executa uma etapa da lavagem pesada.
// saidas define quais acionamentos ficam ativos:
// RB0 = entrada de agua; RB1 = motor; RB2 = drenagem;
// RB3 = centrifugacao.
// Retorna 1 se o usuario cancelar pelo botao VOLTAR.
unsigned char executarEtapaPesada(const char *nomeEtapa,
                                  unsigned char tempo,
                                  unsigned char saidas)
{
    unsigned char segundo;

    // Atualiza os quatro acionamentos sem alterar os bits RB4 a RB7.
    PORTBbits.RB0 = (saidas & 0x01) ? 1 : 0;
    PORTBbits.RB1 = (saidas & 0x02) ? 1 : 0;
    PORTBbits.RB2 = (saidas & 0x04) ? 1 : 0;
    PORTBbits.RB3 = (saidas & 0x08) ? 1 : 0;

    mostrarTela(nomeEtapa, "Ciclo pesado");

    // Divide a espera em intervalos de um segundo para
    // permitir verificar o botao VOLTAR durante a etapa.
    for (segundo = 0; segundo < tempo; segundo++)
    {
        __delay_ms(1000);

        if (BTN_VOLTAR == 0)
        {
            __delay_ms(40); // Debounce do botao

            if (BTN_VOLTAR == 0)
            {
                // Desliga todos os acionamentos antes de cancelar.
                PORTBbits.RB0 = 0;
                PORTBbits.RB1 = 0;
                PORTBbits.RB2 = 0;
                PORTBbits.RB3 = 0;

                // Aguarda a liberacao do botao.
                while (BTN_VOLTAR == 0);
                __delay_ms(40);

                return 1;
            }
        }
    }

    return 0;
}

// Executa o ciclo completo de lavagem pesada.
// Tempos demonstrativos: total de 66 segundos.
void lavagemPesada(void)
{
    // RB0 a RB3 sao saidas; RB4 a RB7 permanecem como entradas.
    TRISB = 0xF0;

    // Garante que todos os acionamentos comecem desligados.
    PORTBbits.RB0 = 0;
    PORTBbits.RB1 = 0;
    PORTBbits.RB2 = 0;
    PORTBbits.RB3 = 0;

    // Etapa 1: pre-lavagem (motor em funcionamento).
    if (executarEtapaPesada("Pre-lavagem", 5, 0x02))
        goto ciclo_cancelado;

    // Etapa 2: enchimento de agua.
    if (executarEtapaPesada("Enchendo agua", 8, 0x01))
        goto ciclo_cancelado;

    // Etapa 3: agitacao mais longa para lavagem pesada.
    if (executarEtapaPesada("Lavagem pesada", 20, 0x02))
        goto ciclo_cancelado;

    // Etapa 4: retirada da agua.
    if (executarEtapaPesada("Drenando agua", 8, 0x04))
        goto ciclo_cancelado;

    // Etapa 5: entrada de agua e movimentacao para enxague.
    if (executarEtapaPesada("Enxaguando", 10, 0x03))
        goto ciclo_cancelado;

    // Etapa 6: drenagem junto com a centrifugacao.
    if (executarEtapaPesada("Centrifugando", 15, 0x0C))
        goto ciclo_cancelado;

    // Desliga todas as saidas ao concluir o ciclo.
    PORTBbits.RB0 = 0;
    PORTBbits.RB1 = 0;
    PORTBbits.RB2 = 0;
    PORTBbits.RB3 = 0;

    mostrarTela("Ciclo concluido", "Lavagem pesada");
    __delay_ms(2000);

    estado = MENU_PRINCIPAL;
    opcao = 0;
    return;

ciclo_cancelado:
    // Mantem todas as saidas desligadas em caso de cancelamento.
    PORTBbits.RB0 = 0;
    PORTBbits.RB1 = 0;
    PORTBbits.RB2 = 0;
    PORTBbits.RB3 = 0;

    mostrarTela("Ciclo cancelado", "Voltando menu");
    __delay_ms(1500);

    estado = MENU_PRINCIPAL;
    opcao = 0;
}


// ==================================================
// SELECAO DE PROGRAMAS
// ==================================================

// Armazena o programa escolhido e informa a selecao.
// A execucao real do ciclo sera implementada depois.
void confirmarPrograma(void)
{
    programaSelecionado = (Programa)opcao;

    switch (programaSelecionado) {
        case RAPIDA:
            mostrarTela("Selecionado:", "Lavagem rapida");
            break;

        case NORMAL:
            mostrarTela("Selecionado:", "Lavagem normal");
            break;

        case PESADA:
            // Executa somente a lavagem pesada; as demais opcoes
            // continuam com o comportamento original do programa.
            mostrarTela("Iniciando ciclo", "Lavagem pesada");
            __delay_ms(800);
            lavagemPesada();
            return; // Evita sobrescrever a tela ao voltar do ciclo.

        case ENXAGUE_CENTRIFUGACAO:
            mostrarTela("Selecionado:", "Enxag+Centrif.");
            break;

        default:
            // Protecao para um indice inesperado.
            mostrarTela("Opcao invalida", "Tente novamente");
            break;
    }

    __delay_ms(1200);

    // Depois da selecao, retorna ao menu principal.
    estado = MENU_PRINCIPAL;
    opcao = 0;
}

// Atualiza a tela conforme o estado atual da interface.
void atualizarTela(void)
{
    switch (estado) {
        case MENU_PRINCIPAL:
            mostrarMenuPrincipal();
            break;

        case MENU_PROGRAMAS:
            mostrarMenuProgramas();
            break;

        case MENU_CONFIGURACOES:
            mostrarConfiguracoes();
            break;

        case MENU_SOBRE:
            mostrarSobre();
            break;

        case MENU_SAIR:
            mostrarTela("Menu encerrado", "Pode desligar PIC");
            break;
    }
}


// ==================================================
// PROGRAMA PRINCIPAL
// ==================================================

void main(void)
{
    // Configura os pinos analogicos como digitais.
    // Necessario para utilizar os pinos correspondentes
    // como entradas e saidas digitais.
    ADCON1 = 0x06;

    // PORTD controla o LCD; PORTC recebe os botoes.
    TRISD = 0x00; // Todos os pinos de PORTD como saida
    TRISC = 0xFF; // Todos os pinos de PORTC como entrada

    PORTD = 0x00;
    PORTC = 0xFF;

    // Inicializa o LCD e apresenta a primeira tela.
    lcdInicializar();
    atualizarTela();

    // Mantem a interface ativa ate selecionar "Sair".
    while (!sair) {

        // -------- BOTAO PROXIMO --------
        if (BTN_PROXIMO == 0) {
            __delay_ms(40); // Debounce: reduz leituras falsas

            if (BTN_PROXIMO == 0) {

                // Avanca uma opcao e retorna ao inicio
                // quando chega ao final da lista.
                if (estado == MENU_PRINCIPAL)
                    opcao = (opcao + 1) % 4;
                else if (estado == MENU_PROGRAMAS)
                    opcao = (opcao + 1) % 4;

                atualizarTela();

                // Aguarda a liberacao para nao repetir
                // a acao continuamente enquanto pressionado.
                while (BTN_PROXIMO == 0);
                __delay_ms(40);
            }
        }

        // -------- BOTAO SELECIONAR --------
        if (BTN_SELECIONAR == 0) {
            __delay_ms(40);

            if (BTN_SELECIONAR == 0) {

                switch (estado) {

                    case MENU_PRINCIPAL:
                        // Abre a tela correspondente ao item.
                        if (opcao == 0) {
                            estado = MENU_PROGRAMAS;
                            opcao = 0;
                        }
                        else if (opcao == 1) {
                            estado = MENU_CONFIGURACOES;
                        }
                        else if (opcao == 2) {
                            estado = MENU_SOBRE;
                        }
                        else if (opcao == 3) {
                            // Encerra a navegacao do menu.
                            estado = MENU_SAIR;
                            sair = 1;
                        }
                        break;

                    case MENU_PROGRAMAS:
                        // Guarda a escolha e retorna ao inicio.
                        confirmarPrograma();
                        break;

                    case MENU_CONFIGURACOES:
                    case MENU_SOBRE:
                        // Essas telas ainda nao possuem
                        // opcoes internas implementadas.
                        estado = MENU_PRINCIPAL;
                        opcao = 0;
                        break;

                    case MENU_SAIR:
                        sair = 1;
                        break;
                }

                atualizarTela();

                while (BTN_SELECIONAR == 0);
                __delay_ms(40);
            }
        }

        // -------- BOTAO VOLTAR --------
        if (BTN_VOLTAR == 0) {
            __delay_ms(40);

            if (BTN_VOLTAR == 0) {

                // Qualquer submenu retorna ao menu principal.
                if (estado != MENU_PRINCIPAL) {
                    estado = MENU_PRINCIPAL;
                    opcao = 0;
                }

                atualizarTela();

                while (BTN_VOLTAR == 0);
                __delay_ms(40);
            }
        }
    }

    // O PIC nao e desligado por software.
    // Permanece parado com a mensagem de encerramento.
    while (1) {
        // Aguarda reset ou desligamento da alimentacao.
    }
}