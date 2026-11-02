
    
  
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