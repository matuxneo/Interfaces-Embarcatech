# EMBARCATECH 2025 -  Tarefa - Comunicação Serial com RP2040 UART, SPI e I2C

# Aluno: Matuzalém Guimarães Leal - matuinfo@gmail.com

## Funcionalidades

1. Modificação da Biblioteca `font.h`

    - Adicionado caracteres minúsculos à biblioteca `font.h`. 

2. Entrada de caracteres via Serial Monitor

    - Use o Serial Monitor do VS Code para digitar os caracteres.
    - Cada caractere digitado no Serial Monitor é exibido no display SSD1306.

        > Observação: Apenas um caractere é enviado de cada vez, não suporta o envio de strings.

    - Quando um número entre 0 e 9 for digitado, um símbolo correspondente ao número deve é  exibido, também, na matriz 5x5 WS2812.

3. Ao pressionar o Botão A

    - Pressionar o botão A deve alternar o estado do LED RGB Verde (ligado/desligado).
    - A operação deve ser registrada de duas formas:
        - Uma mensagem informativa sobre o estado do LED é exibida no display SSD1306
        - Um texto descritivo sobre a operação é enviado ao Serial Monitor.
        
4. Ao Pressionar o Botão B

    - Pressionar o botão B deve alternar o estado do LED RGB Azul (ligado/desligado).
    - A operação deve ser registrada de duas formas:

        - Uma mensagem informativa sobre o estado do LED deve é exibida no display SSD1306
        - Um texto descritivo sobre a operação deve é enviado ao Serial Monitor.

## Requisitos do Projeto

Foram implementados os seguintes requisitos:

1. Uso de interrupções: Todas as funcionalidades relacionadas aos botões devem ser implementadas
utilizando rotinas de interrupção (IRQ).
2. Debouncing: É obrigatório implementar o tratamento do bouncing dos botões via software.
3. Controle de LEDs: O projeto deve incluir o uso de LEDs comuns e LEDs WS2812, demonstrando o domínio de diferentes tipos de controle.
4. Utilização do Display 128 x 64: A utilização de fontes maiúsculas e minúsculas demonstrará o domínio do uso de bibliotecas, o entendimento do princípio de funcionamento do display, bem como, a utilização do protocolo I2C.
5. Envio de informação pela UART: Visa observar a compreensão sobre a comunicação serial via UART.
6. Organização do código: O código deve estar bem estruturado e comentado para facilitar o entendimento.
