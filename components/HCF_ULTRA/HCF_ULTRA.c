#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"

#define TRIG_PIN selecionado_trigger  // Defina o pino TRIG
#define ECHO_PIN selecionado_echo  // Defina o pino ECHO

static const char *TAG = "HCF_ULTRA";

int selecionado_trigger, selecionado_echo;


// Função para configurar os pinos
void iniciar_ultrassonico(int Trigger, int Echo) {
    selecionado_trigger = Trigger;
    selecionado_echo = Echo;
    gpio_set_direction(TRIG_PIN, GPIO_MODE_OUTPUT);
    gpio_set_direction(ECHO_PIN, GPIO_MODE_INPUT);
    gpio_set_level(TRIG_PIN, 0);
}

// Função para medir a distância
float medir_distancia() {
    // Envia o pulso de 10us no TRIG
    gpio_set_level(TRIG_PIN, 1);
    esp_rom_delay_us(10);
    gpio_set_level(TRIG_PIN, 0);

    // Timeout inicial
    int64_t espera_inicial = esp_timer_get_time();
    const int estouro_us = 70000;  // 30ms timeout (ajustável)
    

    // Aguarda o ECHO ir para nível alto
    while (gpio_get_level(ECHO_PIN) == 0) {
        if (esp_timer_get_time() - espera_inicial > estouro_us) {
            ESP_LOGW(TAG, "Timeout esperando ECHO HIGH");
            return -1;  // Retorna valor inválido
        }
    }
    int64_t tempo_inicial = esp_timer_get_time();  // Marca o tempo inicial

    // Aguarda o ECHO ir para nível baixo
        // Espera ECHO ir para LOW
        while (gpio_get_level(ECHO_PIN) == 1) {
            if (esp_timer_get_time() - tempo_inicial > estouro_us) {
                ESP_LOGW(TAG, "Timeout esperando ECHO LOW");
                return -1;  // Retorna valor inválido
            }
        }

    int64_t tempo_final = esp_timer_get_time();  // Marca o tempo final

    // Calcula a duração do pulso em microsegundos
    int64_t periodo = tempo_final - tempo_inicial;

    // Calcula a distância em cm (velocidade do som = 34300 cm/s)
    float distancia = (periodo / 2.0) * 0.0343;

    return distancia;
}


