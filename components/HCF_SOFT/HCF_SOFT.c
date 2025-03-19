#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
//#include "esp_rom.h"
#include <stdio.h>
#include "esp_timer.h"
//#include "esp_rom/esp_rom_functions.h" 
//#include <string.h>
//#include "esp_system.h"
static const char *TAG = "HCF_SOFT";

int espera_nivel(int pino, int nivel, uint32_t estouro_us) {
    uint32_t tempo_inicial = esp_timer_get_time(); // Pega o tempo atual em microssegundos
    while (gpio_get_level(pino) != nivel) {
        if ((esp_timer_get_time() - tempo_inicial) > estouro_us) {
            return 0;  // Timeout
        }
    }
    return 1;  // Sucesso
}
void delay_us (int us)
{
    esp_rom_delay_us(us);
}
void delay_ms (int ms)
{
    vTaskDelay(ms / portTICK_PERIOD_MS); 
}
void piscar_LED(int vezes, int pino, int tempo_alto, int tempo_baixo)
{
    for(int i = 0; i < vezes; i++)
    {
        gpio_set_level(pino, 1);
        delay_ms(tempo_alto);
            
        gpio_set_level(pino, 0);
        delay_ms(tempo_baixo);
    }
}

