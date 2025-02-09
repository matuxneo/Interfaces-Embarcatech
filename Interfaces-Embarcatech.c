#include <stdio.h> // Inclui a biblioteca stdio.h para usar a função printf
#include "pico/stdlib.h" // Inclui a biblioteca pico/stdlib.h para usar as funções de inicialização do Pico
#include "hardware/i2c.h" // Inclui a biblioteca hardware/i2c.h para usar a comunicação I2C
#include "hardware/uart.h" // Inclui a biblioteca hardware/uart.h para usar a comunicação UART
#include "inc/ssd1306.h" // Inclui a biblioteca inc/ssd1306.h para usar o display SSD1306
#include "inc/font.h" // Inclui a biblioteca inc/font.h para usar a fonte do display
#include "hardware/pio.h" // Inclui a biblioteca hardware/pio.h para usar a PIO
#include "ws2818b.pio.h" // Inclui a biblioteca ws2818b.pio.h para usar a PIO para os LEDs

#define LED_PIN 7        // Pino GPIO conectado aos LEDs
#define LED_COUNT 25     // Número de LEDs na matriz
// I2C defines
#define I2C_PORT i2c1 // Porta I2C a ser usada
#define I2C_SDA 14 // Pino SDA do I2C
#define I2C_SCL 15 // Pino SCL do I2C
#define endereco 0x3C // Endereço I2C do display
ssd1306_t SSD; // Inicializa a estrutura do display

// UART defines
#define UART_ID uart0 // UART a ser usado
#define BAUD_RATE 115200 // Taxa de transferência UART
#define UART_TX_PIN 0 // Pino TX do UART
#define UART_RX_PIN 1 // Pino RX do UART

// GPIO defines
#define LED_PIN_G 11    // Pino do verde do LED RGB
#define LED_PIN_B 12   // Pino do azul do LED RGB
const uint BUTTON_A = 5;  // Pino GPIO do botão A
const uint BUTTON_B = 6; // Pino GPIO do botão B


// Variáveis globais
static volatile uint32_t last_time = 0; // Variável para armazenar o tempo do último evento
static volatile bool led_state_G = false, led_state_B = false; // Variáveis para armazenar o estado dos LEDs

// Função para inicializar o display SSD1306
void i2c_initi() 
{   
    i2c_init(I2C_PORT, 400*1000); // Inicializa a porta I2C com 400KHz
    
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);  // Configura o pino SDA como função I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);  // Configura o pino SCL como função I2C
    gpio_pull_up(I2C_SDA); // Habilita o pull-up no pino SDA
    gpio_pull_up(I2C_SCL); // Habilita o pull-up no pino SCL
    
    ssd1306_init(&SSD, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&SSD); // Configura o display
    ssd1306_send_data(&SSD); // Envia os dados para o display

    // Inicializa a matriz de LEDs
    ssd1306_fill(&SSD, false); // Limpa o display
    ssd1306_send_data(&SSD); // Atualiza o display

    bool cor = false; // Variável para alternar a cor do retângulo
    cor = !cor; // Inverte a cor
    ssd1306_fill(&SSD, !cor); // Limpa o display
    ssd1306_rect(&SSD, 3, 3, 122, 58, cor, !cor); // Desenha um retângulo
    ssd1306_draw_string(&SSD, "VERDE OFF", 24, 18); // Desenha uma string
    ssd1306_draw_string(&SSD, "CHAR 0", 24, 30); // Desenha uma string
    ssd1306_draw_string(&SSD, "AZUL OFF", 24, 42);  // Desenha uma string
    ssd1306_send_data(&SSD); // Atualiza o display
}


struct pixel_t // Estrutura para armazenar as cores de cada LED
{
    uint8_t G, R, B; // Componentes de cor: Verde, Vermelho e Azul
};

typedef struct pixel_t pixel_t;  // Definir o tipo pixel_t
typedef pixel_t npLED_t;  // Definir o tipo npLED_t como um pixel_t

npLED_t leds[LED_COUNT]; // Array de LEDs
PIO np_pio;  // Bloco PIO para controle dos LEDs          
uint sm;      // State machine para controle dos LEDs     


// Função para inicializar os LEDs
void npInit(uint pin) // Inicializar os LEDs
{
    uint offset = pio_add_program(pio0, &ws2818b_program); // Carregar o programa PIO
    np_pio = pio0;                                         // Usar o primeiro bloco PIO

    sm = pio_claim_unused_sm(np_pio, false); // Tentar usar uma state machine do pio0
    if (sm < 0)                              // Se não houver disponível no pio0
    {
        np_pio = pio1;                          // Mudar para o pio1
        sm = pio_claim_unused_sm(np_pio, true); // Usar uma state machine do pio1
    }

    ws2818b_program_init(np_pio, sm, offset, pin, 800000.f); // Inicializar state machine para LEDs

  // Inicializar os LEDs com cores padrão
    for (uint i = 0; i < LED_COUNT; ++i) // Iterar sobre todos os LEDs
    {
        leds[i].R = 0; // Definir componente vermelho
        leds[i].G = 0; // Definir componente verde
        leds[i].B = 0; // Definir componente azul
    }
}

// Função para atualizar os LEDs no hardware
void npUpdate() 
{
    for (uint i = 0; i < LED_COUNT; ++i) // Iterar sobre todos os LEDs
    {
        pio_sm_put_blocking(np_pio, sm, leds[i].R); // Enviar componente vermelho
        pio_sm_put_blocking(np_pio, sm, leds[i].G); // Enviar componente verde        
        pio_sm_put_blocking(np_pio, sm, leds[i].B); // Enviar componente azul
    }
}


// Função para definir a cor de um LED específico
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b)
{
    leds[index].R = r; // Definir componente vermelho
    leds[index].G = g; // Definir componente verde
    leds[index].B = b; // Definir componente azul
}


// Função para limpar (apagar) todos os LEDs
void npClear()
{
    for (uint i = 0; i < LED_COUNT; ++i) // Iterar sobre todos os LEDs
        npSetLED(i, 0, 0, 0);            // Definir cor como preta (apagado)
}


// Função para definir qual número mostrar na tela
void setDisplayNum(char num, const uint8_t r, const uint8_t g, const uint8_t b)
{
/*  Gabarito do Display
    24, 23, 22, 21, 20
    15, 16, 17, 18, 19
    14, 13, 12, 11, 10
    05, 06, 07, 08, 09
    04, 03, 02, 01, 00
*/
    npClear();
    switch (num)
    {
    case '0': // Número 0
        npSetLED(1, r, g, b);
        npSetLED(2, r, g, b);
        npSetLED(3, r, g, b);
        npSetLED(6, r, g, b);
        npSetLED(8, r, g, b);
        npSetLED(11, r, g, b);
        npSetLED(13, r, g, b);
        npSetLED(16, r, g, b);
        npSetLED(18, r, g, b);        
        npSetLED(21, r, g, b);
        npSetLED(22, r, g, b);
        npSetLED(23, r, g, b);
        break;
    case '1': // Número 1
        npSetLED(1, r, g, b);
        npSetLED(2, r, g, b);
        npSetLED(3, r, g, b);
        npSetLED(7, r, g, b);
        npSetLED(12, r, g, b);
        npSetLED(16, r, g, b);
        npSetLED(17, r, g, b);
        npSetLED(22, r, g, b);
        break;
    case '2': // Número 2
        npSetLED(1, r, g, b);
        npSetLED(2, r, g, b);
        npSetLED(3, r, g, b);
        npSetLED(6, r, g, b);
        npSetLED(11, r, g, b);
        npSetLED(12, r, g, b);
        npSetLED(13, r, g, b);
        npSetLED(18, r, g, b);
        npSetLED(21, r, g, b);
        npSetLED(22, r, g, b);
        npSetLED(23, r, g, b);
        break;
    case '3': // Número 3
        npSetLED(1, r, g, b);
        npSetLED(2, r, g, b);
        npSetLED(3, r, g, b);
        npSetLED(8, r, g, b);
        npSetLED(11, r, g, b);
        npSetLED(12, r, g, b);
        npSetLED(13, r, g, b);
        npSetLED(18, r, g, b);
        npSetLED(21, r, g, b);
        npSetLED(22, r, g, b);
        npSetLED(23, r, g, b);
        break;
    case '4': // Número 4
        npSetLED(2, r, g, b);
        npSetLED(5, r, g, b);
        npSetLED(6, r, g, b);
        npSetLED(7, r, g, b);
        npSetLED(8, r, g, b);
        npSetLED(12, r, g, b);
        npSetLED(14, r, g, b);
        npSetLED(16, r, g, b);
        npSetLED(17, r, g, b);
        npSetLED(22, r, g, b);
        break;
    case '5': // Número 5
        npSetLED(1, r, g, b);
        npSetLED(2, r, g, b);
        npSetLED(3, r, g, b);        
        npSetLED(8, r, g, b);
        npSetLED(11, r, g, b);
        npSetLED(12, r, g, b);
        npSetLED(13, r, g, b);
        npSetLED(16, r, g, b); 
        npSetLED(21, r, g, b);
        npSetLED(22, r, g, b);
        npSetLED(23, r, g, b);
        break;
    case '6': // Número 6
        npSetLED(1, r, g, b);
        npSetLED(2, r, g, b);
        npSetLED(3, r, g, b); 
        npSetLED(6, r, g, b);
        npSetLED(8, r, g, b);
        npSetLED(11, r, g, b);
        npSetLED(12, r, g, b);
        npSetLED(13, r, g, b);
        npSetLED(16, r, g, b); 
        npSetLED(21, r, g, b);
        npSetLED(22, r, g, b);
        npSetLED(23, r, g, b);
        break;
    case '7': // Número 7
        npSetLED(1, r, g, b);
        npSetLED(8, r, g, b);
        npSetLED(11, r, g, b);
        npSetLED(18, r, g, b);
        npSetLED(21, r, g, b);
        npSetLED(22, r, g, b);
        npSetLED(23, r, g, b);        
        break;
    case '8': // Número 8
        npSetLED(1, r, g, b);
        npSetLED(2, r, g, b);
        npSetLED(3, r, g, b);
        npSetLED(6, r, g, b);
        npSetLED(8, r, g, b);
        npSetLED(11, r, g, b);
        npSetLED(12, r, g, b);
        npSetLED(13, r, g, b);
        npSetLED(16, r, g, b);
        npSetLED(18, r, g, b);
        npSetLED(22, r, g, b);
        npSetLED(21, r, g, b);
        npSetLED(23, r, g, b);
        break;
    case '9': // Número 9
        npSetLED(1, r, g, b);
        npSetLED(2, r, g, b);
        npSetLED(3, r, g, b);        
        npSetLED(8, r, g, b);
        npSetLED(11, r, g, b);
        npSetLED(12, r, g, b);
        npSetLED(13, r, g, b);
        npSetLED(16, r, g, b);
        npSetLED(18, r, g, b);
        npSetLED(21, r, g, b);
        npSetLED(22, r, g, b);
        npSetLED(23, r, g, b);
        break;
    } // default desnecessário
    npUpdate(); // Atualiza o display após colocar o número
}

/*=====================================Interrupções=====================================*/

// Função de interrupção IRQ com debouncing dos botões A e B
void gpio_irq_handler(uint gpio, uint32_t events)
{
    // Obtém o tempo atual em microssegundos
    uint32_t current_time = to_us_since_boot(get_absolute_time());
    
    // Verifica se passou tempo suficiente desde o último evento
    if (current_time - last_time > 300000) // 300 ms de debouncing
    {        
        if (gpio == BUTTON_A) // Botão A pressionado
        {
            printf(">> Botão A pressionado! << \n");
            led_state_G = !led_state_G; // Inverte o estado do LED verde
            gpio_put(LED_PIN_G, led_state_G); // Atualiza o estado do LED verde

            // Enviar o que acontece para o display
            if (led_state_G) // Se o LED está ligado
            {
                ssd1306_draw_string(&SSD, "VERDE ON ", 24, 18);  // Desenha a string "VERDE ON" na posição (24, 18)
                printf(">> LED VERDE LIGADO... << \033[32m\u25CF\033[0m\n\n\n"); // Envia mensagem para a UART

            }
            else /// Se o LED está desligado
            {
                ssd1306_draw_string(&SSD, "VERDE OFF", 24, 18);  // Desenha a string "VERDE OFF" na posição (24, 18)
                printf(">> LED VERDE DESLIGADO... << \033[30m\u25CF\033[0m\n\n\n"); // Envia mensagem para a UART
            }            
            ssd1306_send_data(&SSD); // Atualiza o display
            
            last_time = current_time; // Atualiza o tempo do último evento
        }
        else // Botão B pressionado
        {
            printf(">> Botão B pressionado! <<  \n"); // Envia mensagem para a UART
            led_state_B = !led_state_B; // Inverte o estado do LED azul
            gpio_put(LED_PIN_B, led_state_B); // Atualiza o estado do LED azul

        
            // Enviar o que acontece para o display
            if (led_state_B) // Se o LED está ligado
            {
                ssd1306_draw_string(&SSD, "AZUL ON ", 24, 42);  // Desenha a string "AZUL ON" na posição (24, 42)
                printf(">> LED AZUL LIGADO... << \x1B[38;2;0;0;255m\u25CF\x1B[0m\n\n"); // Envia mensagem para a UART
            }
            else //// Se o LED está desligado
            {
                ssd1306_draw_string(&SSD, "AZUL OFF", 24, 42);  // Desenha a string "AZUL OFF" na posição (24, 42)
                printf(">> LED AZUL DESLIGADO... << \033[30m\u25CF\033[0m\n\n\n"); // Envia mensagem para a UART
            }
            ssd1306_send_data(&SSD); // Atualiza o display
            
            last_time = current_time; // Atualiza o tempo do último evento
        }
    }
}

/*=====================================Main=====================================*/
int main()
{
    stdio_init_all(); // Inicializar a comunicação serial

    npInit(LED_PIN);  // Inicializar os LEDs

    // Inicializar o pino GPIO13
    gpio_init(LED_PIN_G); // Inicializa o pino do LED verde
    gpio_set_dir(LED_PIN_G, true); // Configura o pino como saída

    gpio_init(LED_PIN_B); // Inicializa o pino do LED azul
    gpio_set_dir(LED_PIN_B, true);  // Configura o pino como saída

    gpio_init(BUTTON_A); // Inicializa o pino do botão A
    gpio_set_dir(BUTTON_A, GPIO_IN); // Configura o pino como entrada
    gpio_pull_up(BUTTON_A);          // Habilita o pull-up interno
    // Configuração da interrupção com callback do botão A
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);     // Configuração da interrupção com callback do botão A

    gpio_init(BUTTON_B); // Inicializa o pino do botão B
    gpio_set_dir(BUTTON_B, GPIO_IN); // Configura o pino como entrada
    gpio_pull_up(BUTTON_B);          // Habilita o pull-up interno
    // Configuração da interrupção com callback do botão B
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler); // Configuração da interrupção com callback do botão B



    uart_init(UART_ID, BAUD_RATE); // Inicializa a UART
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART); // Configura o pino TX como UART
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);  // Configura o pino RX como UART
    

    uart_puts(UART_ID, ">> Inicializando o UART... \n"); // Envia mensagem de inicialização
        
    i2c_initi(); // Inicializa o display SSD1306
    while (true) { //                        
        if (stdio_usb_connected()) // Se o USB está conectado
        { 
            printf("> "); // Imprime o prompt
            char c; // Variável para armazenar o caractere lido
            if (scanf("%c", &c) == 1) // Se o caractere foi lido com sucesso
            { // Lê caractere da entrada padrão
                ssd1306_draw_char(&SSD, c, 64, 30); // Desenha o caractere na posição (64, 30)
                ssd1306_send_data(&SSD); // Atualiza o display
                setDisplayNum(c, 50, 30, 30); // Define o número a ser exibido                
            } // Se o caractere não foi lido com sucesso
        } else  {
            printf("Verifique se a placa está conectada a uma porta USB neste dispositivo...\n");// Se o USB não está conectado       
    }
    } // Fim do loop infinito
}
