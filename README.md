# 🧺 Washing Machine Simulator

> Uma simulação em C da lógica de funcionamento de uma máquina de lavar,
> explorando controle de estados, sequenciamento de ciclos e fundamentos
> de sistemas embarcados --- sem necessidade de hardware físico.

**Status:** 🚧 Em desenvolvimento\
**Linguagem:** C\
**Ambiente:** MPLAB X IDE

## Visão geral

O **Washing Machine Simulator** é um projeto acadêmico que propõe
representar, por software, o funcionamento de uma máquina de lavar
roupas. Em vez de controlar componentes físicos, o sistema modela as
etapas de um ciclo de lavagem e suas transições, permitindo explorar
como a lógica de controle de um equipamento pode ser estruturada e
testada.

O projeto conecta conceitos de programação em C a ideias comuns em
sistemas embarcados, com foco na organização do fluxo de execução, no
controle de estados e no tratamento das etapas de um processo
automatizado.

## O que a simulação pretende demonstrar

-   **Ciclo de lavagem estruturado:** representação das principais fases
    de funcionamento.
-   **Controle por estados:** cada etapa possui um estado definido e
    regras para avançar à próxima.
-   **Sequenciamento lógico:** as fases seguem uma ordem coerente,
    conforme as regras estabelecidas.
-   **Tratamento de entradas:** previsão de validação de comandos e
    opções selecionadas.
-   **Verificação do comportamento:** testes de diferentes caminhos e
    situações para conferir a lógica do sistema.

## Como funcionará

A proposta é representar um ciclo de lavagem por etapas, como:

`Início → Enchimento → Lavagem → Drenagem → Enxágue → Centrifugação → Conclusão`

Cada etapa será modelada logicamente no programa. As condições de
transição e a forma de interação com o usuário serão definidas durante a
implementação.

Esse fluxo é ilustrativo: a sequência final dependerá dos requisitos
adotados para o projeto.

## Tecnologias e conceitos

  -----------------------------------------------------------------------
  Tecnologia / conceito               Aplicação no projeto
  ----------------------------------- -----------------------------------
  **C**                               Implementação da lógica de
                                      simulação

  **MPLAB X IDE**                     Ambiente de desenvolvimento
                                      previsto

  **Máquina de estados**              Organização das etapas e transições
                                      do ciclo

  **Lógica de controle**              Definição das condições de execução

  **Testes de software**              Verificação de cenários e
                                      comportamentos esperados

  **Git e GitHub**                    Versionamento e publicação do
                                      projeto
  -----------------------------------------------------------------------

> **Nota técnica:** o MPLAB X IDE é voltado principalmente ao
> desenvolvimento para microcontroladores. Como esta versão não utiliza
> hardware físico, a configuração de compilação e execução deverá ser
> compatível com a abordagem de simulação escolhida.

## Por que este projeto?

A lógica de um equipamento automatizado envolve mais do que executar
ações em sequência: é necessário representar estados, controlar
transições e prever comportamentos para diferentes condições. Este
projeto utiliza uma máquina de lavar como exemplo acessível para estudar
esses fundamentos de forma organizada e testável.

## Escopo

O projeto tem como foco a **simulação lógica por software**. Não inclui,
nesta etapa, integração com sensores, motores, bombas, válvulas ou
outros componentes físicos. O comportamento final dependerá das
funcionalidades implementadas ao longo do desenvolvimento.

## Estado atual

O repositório está na fase inicial de planejamento e definição do
comportamento esperado. As funcionalidades descritas são objetivos de
desenvolvimento, não recursos já concluídos. O código e as instruções de
execução serão adicionados conforme a implementação evoluir.

## Próximos marcos

-   [ ] Definição do fluxo e dos estados do ciclo
-   [ ] Implementação da lógica de simulação em C
-   [ ] Definição da interface de entrada e saída
-   [ ] Testes dos estados e das transições
-   [ ] Documentação de compilação e execução

------------------------------------------------------------------------

*Projeto acadêmico de simulação de uma máquina de lavar em C.*
