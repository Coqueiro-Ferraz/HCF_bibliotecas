#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"

static const char *TAG = "HCF_DHT";
#define DHT_PIN pino_dados  // Pino de dados do DHT22
#define DHT_ESTOURO 1000    // Timeout em microssegundos para leitura de pulso

int pino_dados;

// Função para inicializar o sensor DHT22
void iniciar_DHT(int pin) {
    pino_dados = pin;
    gpio_reset_pin(DHT_PIN);
    gpio_set_direction(DHT_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(DHT_PIN, 1); // Coloca o pino em estado alto
    
    vTaskDelay(pdMS_TO_TICKS(1)); // Aguarda 1 ms
}

// Função para enviar o pulso de início
void DHT_enviar_start() {
    
    gpio_set_direction(DHT_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(DHT_PIN, 1); 
    vTaskDelay(pdMS_TO_TICKS(1));

    gpio_set_level(DHT_PIN, 0); // Coloca o pino em estado baixo
    vTaskDelay(pdMS_TO_TICKS(18)); // Aguarda 18 ms <--- muda conforme o sensor
    gpio_set_level(DHT_PIN, 1); // Retorna para estado alto
    esp_rom_delay_us(30);       // Espera 30 µs
}

// Função para esperar até que o pino mude de estado ou ocorra timeout
int espera_nivel(int nivel, uint32_t estouro_us) {
    uint32_t tempo_inicial = esp_timer_get_time(); // Pega o tempo atual em microssegundos
    while (gpio_get_level(DHT_PIN) != nivel) {
        if ((esp_timer_get_time() - tempo_inicial) > estouro_us) {
            return 0;  // Timeout
        }
    }
    return 1;  // Sucesso
}
// Função para ler os dados brutos do sensor (40 bits)
int DHT_ler_serial(uint8_t dados[5]) {
    // Configura o pino como entrada para ler os dados
    gpio_set_direction(DHT_PIN, GPIO_MODE_INPUT);

    // Espera pelo pulso de resposta do sensor
    if (!espera_nivel(0, DHT_ESTOURO)) return 0;
    if (!espera_nivel(1, DHT_ESTOURO)) return 0;
    if (!espera_nivel(0, DHT_ESTOURO)) return 0;

    // Lê os 40 bits (5 bytes)
    for (int i = 0; i < 40; i++) {
        if (!espera_nivel(1, DHT_ESTOURO)) return 0; // Aguarda o pulso de início
        int tempo_pulso = esp_timer_get_time();         // Captura o tempo em que o pulso começou
        if (!espera_nivel(0, DHT_ESTOURO)) return 0; // Aguarda o fim do pulso
        tempo_pulso = esp_timer_get_time() - tempo_pulso; // Calcula a duração do pulso

        // Se o pulso for mais longo que 40 µs, é um '1', caso contrário, é '0'
        dados[i / 8] <<= 1; // Desloca para armazenar o bit
        if (tempo_pulso > 40) {
            dados[i / 8] |= 1;
        }
    }

    return 1;  // Leitura bem-sucedida
}

// Função para calcular a umidade e temperatura a partir dos dados lidos
int DHT_temp_umidade(float *temperatura, float *umidade) {
    uint8_t dados[5] = {0};  // Array para armazenar os 5 bytes lidos

    DHT_enviar_start(); // Envia o pulso de início

    if (!DHT_ler_serial(dados)) {
        ESP_LOGW(TAG,"Erro ao ler dados do sensor!");
        return 0;  // Erro de leitura
    }

    // Verifica a soma de verificação (checksum)
    if (dados[4] != ((dados[0] + dados[1] + dados[2] + dados[3]) & 0xFF)) {
        ESP_LOGW(TAG,"Checksum inválido!");
        return 0;  // Erro no checksum
    }

    // Calcula a umidade (primeiros 2 bytes)
    *umidade = ((dados[0] << 8) | dados[1]) * 0.1;

    // Calcula a temperatura (próximos 2 bytes)
    *temperatura = ((dados[2] & 0x7F) << 8 | dados[3]) * 0.1;
    if (dados[2] & 0x80) {  // Verifica o bit de sinal (para temperaturas negativas)
        *temperatura *= -1;
    }

    return 1;  // Leitura bem-sucedida
}