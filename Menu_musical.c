#include <stdio.h>
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "lib/ssd1306.h"
#include "hardware/adc.h"
#include "hardware/i2c.h"
#include "hardware/pwm.h"
#include "string.h" 

// Definição do endereço I2C do display OLED
#define OLED_ADDR 0x3C
#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15

// Definição dos pinos dos botões
#define BUTTON_A_PIN 5
#define BUTTON_B_PIN 6
#define JOYSTICK_PB 22

// Definição dos pinos do joystick
#define JOYSTICK_X_PIN 26
#define JOYSTICK_Y_PIN 27

// Definição do pino do buzzer
#define BUZZER_PIN_1 10
#define BUZZER_PIN_2 21


// Definição da faixa de zona morta (joystick solto)
#define DEADZONE_MIN 250
#define DEADZONE_MAX 3800

// Definição das dimensões do display
#define WIDTH 128
#define HEIGHT 64

// Número total de notas
#define NUM_NOTES 7
// Número de notas visíveis
#define VISIBLE_NOTES 10

typedef struct {
    const char *nome;
    int oitava;
    int frequencia;
    int frequencia_sustenido;
    int frequencia_bemol;
  } Nota;
  
  Nota notas[] = {
    {"DO", 0, 32, 34, 30},   // DO 0, DO# 0, SIb -1
    {"RE", 0, 36, 38, 34},   // RE 0, RE# 0, DO# 0
    {"MI", 0, 41, 43, 38},   // MI 0, FA 0, RE# 0
    {"FA", 0, 43, 46, 41},   // FA 0, FA# 0, MI 0
    {"SOL", 0, 48, 51, 46},  // SOL 0, SOL# 0, FA# 0
    {"LA", 0, 55, 58, 51},   // LA 0, LA# 0, SOL# 0
    {"SI", 0, 61, 65, 58},   // SI 0, DO 1, LA# 0
  
    {"DO", 1, 65, 69, 61},   // DO 1, DO# 1, SIb 0
    {"RE", 1, 73, 77, 69},   // RE 1, RE# 1, DO# 1
    {"MI", 1, 82, 87, 77},   // MI 1, FA 1, RE# 1
    {"FA", 1, 87, 92, 82},   // FA 1, FA# 1, MI 1
    {"SOL", 1, 98, 104, 92}, // SOL 1, SOL# 1, FA# 1
    {"LA", 1, 110, 116, 104},// LA 1, LA# 1, SOL# 1
    {"SI", 1, 123, 130, 116},// SI 1, DO 2, LA# 1
  
    {"DO", 2, 130, 138, 123},// DO 2, DO# 2, SIb 1
    {"RE", 2, 146, 155, 138},// RE 2, RE# 2, DO# 2
    {"MI", 2, 164, 174, 155},// MI 2, FA 2, RE# 2
    {"FA", 2, 174, 185, 164},// FA 2, FA# 2, MI 2
    {"SOL", 2, 196, 207, 185},// SOL 2, SOL# 2, FA# 2
    {"LA", 2, 220, 233, 207},// LA 2, LA# 2, SOL# 2
    {"SI", 2, 246, 261, 233},// SI 2, DO 3, LA# 2
  
    {"DO", 3, 261, 277, 246},// DO 3, DO# 3, SIb 2
    {"RE", 3, 293, 311, 277},// RE 3, RE# 3, DO# 3
    {"MI", 3, 329, 349, 311},// MI 3, FA 3, RE# 3
    {"FA", 3, 349, 370, 329},// FA 3, FA# 3, MI 3
    {"SOL", 3, 392, 415, 370},// SOL 3, SOL# 3, FA# 3
    {"LA", 3, 440, 466, 415},// LA 3, LA# 3, SOL# 3
    {"SI", 3, 493, 523, 466},// SI 3, DO 4, LA# 3
  
    {"DO", 4, 523, 554, 493},// DO 4, DO# 4, SIb 3
    {"RE", 4, 587, 622, 554},// RE 4, RE# 4, DO# 4
    {"MI", 4, 659, 698, 622},// MI 4, FA 4, RE# 4
    {"FA", 4, 698, 740, 659},// FA 4, FA# 4, MI 4
    {"SOL", 4, 783, 830, 740},// SOL 4, SOL# 4, FA# 4
    {"LA", 4, 880, 932, 830},// LA 4, LA# 4, SOL# 4
    {"SI", 4, 987, 1046, 932},// SI 4, DO 5, LA# 4
  
    {"DO", 5, 1046, 1108, 987},// DO 5, DO# 5, SIb 4
    {"RE", 5, 1174, 1244, 1108},// RE 5, RE# 5, DO# 5
    {"MI", 5, 1318, 1396, 1244},// MI 5, FA 5, RE# 5
    {"FA", 5, 1396, 1480, 1318},// FA 5, FA# 5, MI 5
    {"SOL", 5, 1567, 1661, 1480},// SOL 5, SOL# 5, FA# 5
    {"LA", 5, 1760, 1864, 1661},// LA 5, LA# 5, SOL# 5
    {"SI", 5, 1975, 2093, 1864},// SI 5, DO 6, LA# 5
  
    {"DO", 6, 2093, 2217, 1975},// DO 6, DO# 6, SIb 5
    {"RE", 6, 2349, 2489, 2217},// RE 6, RE# 6, DO# 6
    {"MI", 6, 2637, 2793, 2489},// MI 6, FA 6, RE# 6
    {"FA", 6, 2793, 2960, 2637},// FA 6, FA# 6, MI 6
    {"SOL", 6, 3135, 3322, 2960},// SOL 6, SOL# 6, FA# 6
    {"LA", 6, 3520, 3729, 3322},// LA 6, LA# 6, SOL# 6
    {"SI", 6, 3951, 4186, 3729},// SI 6, DO 7, LA# 6
  
    {"DO", 7, 4186, 4434, 3951},// DO 7, DO# 7, SIb 6
    {"RE", 7, 4698, 4978, 4434},// RE 7, RE# 7, DO# 7
    {"MI", 7, 5274, 5587, 4978},// MI 7, FA 7, RE# 7
    {"FA", 7, 5587, 5919, 5274},// FA 7, FA# 7, MI 7
    {"SOL", 7, 6271, 6644, 5919},// SOL 7, SOL# 7, FA# 7
    {"LA", 7, 7040, 7458, 6644},// LA 7, LA# 7, SOL# 7
    {"SI", 7, 7902, 8372, 7458},// SI 7, DO 8, LA# 7
  };
  int NUM_NOTAS = sizeof(notas) / sizeof(notas[0]);

// Variáveis de estado
int cursor_pos = 0; // Posição do cursor (0 a VISIBLE_NOTES - 1)
int scroll_index = 0; // Índice de rolagem (0 a NUM_NOTES - VISIBLE_NOTES)
bool nota_tocando = false; // Indica se uma nota está tocando no momento
int nota_atual = -1; // Índice da nota atual sendo tocada
bool botao_joystick_pressionado = false; // Estado do botão do joystick
int frequencia_atual = 0; // Frequência atual do buzzer
bool botao_a_pressionado = false; // Estado do botão A
bool botao_b_pressionado = false; // Estado do botão B
volatile uint32_t last_button_a_press = 0;
volatile uint32_t last_button_b_press = 0;

// Função para configurar o PWM no pino do buzzer
void configurar_pwm() {
    gpio_set_function(BUZZER_PIN_1, GPIO_FUNC_PWM);
    gpio_set_function(BUZZER_PIN_2, GPIO_FUNC_PWM);

    uint slice_num_1 = pwm_gpio_to_slice_num(BUZZER_PIN_1);
    uint slice_num_2 = pwm_gpio_to_slice_num(BUZZER_PIN_2);

    pwm_set_clkdiv(slice_num_1, 125); // Define o divisor de clock para 125 (1 MHz)
    pwm_set_clkdiv(slice_num_2, 125);

    pwm_set_wrap(slice_num_1, 1000); // Define o valor de wrap inicial
    pwm_set_wrap(slice_num_2, 1000);

    pwm_set_enabled(slice_num_1, true);
    pwm_set_enabled(slice_num_2, true);
}

// Função para tocar uma nota no buzzer usando PWM
void tocar_nota(int frequencia) {
    if (frequencia == 0) {
        pwm_set_gpio_level(BUZZER_PIN_1, 0);
        pwm_set_gpio_level(BUZZER_PIN_2, 0);
        return;
    }

    uint slice_num_1 = pwm_gpio_to_slice_num(BUZZER_PIN_1);
    uint slice_num_2 = pwm_gpio_to_slice_num(BUZZER_PIN_2);

    uint32_t wrap = 1000000 / frequencia;

    pwm_set_wrap(slice_num_1, wrap);
    pwm_set_wrap(slice_num_2, wrap);

    pwm_set_gpio_level(BUZZER_PIN_1, wrap / 2);
    pwm_set_gpio_level(BUZZER_PIN_2, wrap / 2);
}

// Função de interrupção para o botão A (bemol)
void button_a_isr() {
    botao_a_pressionado = !botao_a_pressionado;
}

// Função de interrupção para o botão B (sustenido)
void button_b_isr() {
    botao_b_pressionado = !botao_b_pressionado;
}


int main() {
    // Inicializações
    stdio_init_all();
    gpio_init(BUTTON_A_PIN);
    gpio_init(BUTTON_B_PIN);
    gpio_init(JOYSTICK_PB);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_set_dir(JOYSTICK_PB, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);
    gpio_pull_up(BUTTON_B_PIN);
    gpio_pull_up(JOYSTICK_PB);

    // Inicialização do I2C para o display
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    ssd1306_t display;
    ssd1306_init(&display, 128, 64, false, OLED_ADDR, I2C_PORT);
    ssd1306_config(&display);
    ssd1306_fill(&display, false);
    ssd1306_send_data(&display);

    // Configura o ADC para o joystick
    adc_init();
    adc_gpio_init(JOYSTICK_X_PIN);
    adc_gpio_init(JOYSTICK_Y_PIN);

    // Configura o PWM no pino do buzzer
    configurar_pwm();

    // Configura as interrupções para os botões A e B
    gpio_set_irq_enabled_with_callback(BUTTON_A_PIN, GPIO_IRQ_EDGE_FALL, true, &button_a_isr);
    gpio_set_irq_enabled_with_callback(BUTTON_B_PIN, GPIO_IRQ_EDGE_FALL, true, &button_b_isr);

    // Loop principal
    while (true) {
        // Leitura do valor do eixo X e Y do joystick
        adc_select_input(0); // Eixo X
        uint16_t adc_value_x = adc_read();
        adc_select_input(1); // Eixo Y
        uint16_t adc_value_y = adc_read();

        char nota_atual_nome[8] = ""; // Array para armazenar o nome da nota atual

        // Movimento vertical do cursor
        if (adc_value_x < DEADZONE_MIN) { // Joystick para cima (agora move para baixo)
            if (cursor_pos < VISIBLE_NOTES - 1 && scroll_index + cursor_pos < NUM_NOTAS - 1) {
                cursor_pos++;
            } else if (scroll_index < NUM_NOTAS - VISIBLE_NOTES) {
                scroll_index++;
            }
            sleep_ms(100); // Debounce
        } else if (adc_value_x > DEADZONE_MAX) { // Joystick para baixo (agora move para cima)
            if (cursor_pos > 0) {
                cursor_pos--;
            } else if (scroll_index > 0) {
                scroll_index--;
            }
            sleep_ms(100); // Debounce
        }

        //  Atualiza a nota atual e a frequência se a nota estiver tocando
        if (nota_tocando) {
            nota_atual = scroll_index + cursor_pos;
            frequencia_atual = notas[nota_atual].frequencia;
            memset(nota_atual_nome, 0, sizeof(nota_atual_nome)); // Limpa o nome da nota
            sprintf(nota_atual_nome, "%s", notas[nota_atual].nome); // Atualiza o nome da nota
        }

        // Verifica se o botão do joystick foi pressionado
        if (!gpio_get(JOYSTICK_PB) && !botao_joystick_pressionado) {
            botao_joystick_pressionado = true;
            nota_tocando = !nota_tocando; // Alterna o estado de tocar a nota
        } else if (gpio_get(JOYSTICK_PB)) {
            botao_joystick_pressionado = false;
        }


        // Atualiza a frequência com base nos botões A e B
        if (nota_tocando && nota_atual != -1) {
            uint32_t current_time = to_ms_since_boot(get_absolute_time());
            // Botão A (bemol)
                if (!gpio_get(BUTTON_A_PIN)) {
                    if (current_time - last_button_a_press > 100) { // Debounce
                        frequencia_atual = notas[nota_atual].frequencia_bemol;
                        last_button_a_press = current_time;    
                    }
                } else {
                    last_button_a_press = current_time;
                }
                // Botão B (sustenido)
                if (!gpio_get(BUTTON_B_PIN)) {
                    if (current_time - last_button_b_press > 100) { // Debounce
                        frequencia_atual = notas[nota_atual].frequencia_sustenido;
                        last_button_b_press = current_time;  
                    }
                } else {
                    last_button_b_press = current_time;
                    }
            }

            if (nota_tocando && nota_atual != -1) {
                memset(nota_atual_nome, 0, sizeof(nota_atual_nome)); // Limpa o nome da nota
                if (!gpio_get(BUTTON_A_PIN)) { // Botão A pressionado (nível baixo)
                    frequencia_atual = notas[nota_atual].frequencia_bemol; // Toca o bemol
                    sprintf(nota_atual_nome, "%s b", notas[nota_atual].nome); // Atualiza o nome da nota
                } else if (!gpio_get(BUTTON_B_PIN)) { // Botão B pressionado (nível baixo)
                    frequencia_atual = notas[nota_atual].frequencia_sustenido; // Toca o sustenido
                    sprintf(nota_atual_nome, "%s #", notas[nota_atual].nome); // Atualiza o nome da nota
                } else {
                    frequencia_atual = notas[nota_atual].frequencia; // Volta à nota original
                    sprintf(nota_atual_nome, "%s", notas[nota_atual].nome); // Atualiza o nome da nota
                }
            }

        // Limpa o display
        ssd1306_fill(&display, false);

        // Desenha o layout do menu (cabeçalho e linhas fixas)
        ssd1306_rect(&display, 1, 1, 127, 63, true, false);
        ssd1306_line(&display, 1, 13, 127, 13, true);
        // ssd1306_line(&display, 80, 13, 80, 63, true);
        ssd1306_line(&display, 50, 13, 50, 63, true);

        ssd1306_draw_string(&display, "MENU NOTAS", 25, 4);
        ssd1306_draw_string(&display, "NOTA", 70, 16);
        ssd1306_draw_string(&display, "ATUAL", 67, 24);

            // Desenha a nota atual com o símbolo correspondente
        ssd1306_draw_string(&display, nota_atual_nome, 70, 40);

        // Desenha as notas no menu com rolagem
        for (int i = 0; i < VISIBLE_NOTES; i++) {
            int note_index = scroll_index + i;
            if (note_index < NUM_NOTAS) {
                // Desenha a nota e a oitava
                char nota_com_oitava[8];
                sprintf(nota_com_oitava, "%s %d", notas[note_index].nome, notas[note_index].oitava);
                ssd1306_draw_string(&display, nota_com_oitava, 10, 16 + i * 9);
        
                // Desenha o cursor '>' ao lado da nota selecionada
                if (i == cursor_pos) {
                    ssd1306_draw_string(&display, ">", 2, 16 + i * 9);
                }
            }
            // Limpa a área onde o nome da nota será exibido
            
        }
        

        // Atualiza o display
        ssd1306_send_data(&display);

        // Toca a nota atual no buzzer (se estiver tocando)
        if (nota_tocando) {
            tocar_nota(frequencia_atual); // Toca a nota correspondente
        } else {
            tocar_nota(0); // Desliga o buzzer
        }

        // Pequeno delay para evitar leituras muito rápidas
        sleep_ms(10);
    }
}