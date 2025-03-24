#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <stdio.h>
#include "HCF_IOTEC.h"

#define MP1 GPIO_NUM_19
#define MP2 GPIO_NUM_21
#define MP3 GPIO_NUM_22
#define MP4 GPIO_NUM_23

#define MP_SLEEP GPIO_NUM_22
#define MP_STEP GPIO_NUM_21
#define MP_DIR GPIO_NUM_19

// Define características do motor
#define PULSOS_POR_VOLTA 200  // Motor NEMA17 típico sem microstepping
#define MICRO_PASSOS 2        

#define TMP 10
#define DRV1 io_le_escreve((outputs|0b10000000)&0b10001111)
#define DRV2 io_le_escreve((outputs|0b11000000)&0b11001111)
#define DRV3 io_le_escreve((outputs|0b01000000)&0b01001111)
#define DRV4 io_le_escreve((outputs|0b01100000)&0b01101111)
#define DRV5 io_le_escreve((outputs|0b00100000)&0b00101111)
#define DRV6 io_le_escreve((outputs|0b00110000)&0b00111111)
#define DRV7 io_le_escreve((outputs|0b00010000)&0b00011111)
#define DRV8 io_le_escreve((outputs|0b10010000)&0b10011111)
#define DRV0 io_le_escreve(outputs&0b00001111)

#define FI (1<<FC_INI)
#define FF (1<<FC_FIN)
static const char *TAG = "HCF_MP";
int FC_FIN, FC_INI; 
uint8_t outputs;
static int DRV8825 = 0;

void passo_1()
{
    gpio_set_level(MP1,0);
    gpio_set_level(MP2,1);
    gpio_set_level(MP3,1);
    gpio_set_level(MP4,0);
    vTaskDelay(TMP / portTICK_PERIOD_MS); 
}
void passo_2()
{
    gpio_set_level(MP1,0);
    gpio_set_level(MP2,0);
    gpio_set_level(MP3,1);
    gpio_set_level(MP4,1);
    vTaskDelay(TMP / portTICK_PERIOD_MS); 
}
void passo_3()
{
    gpio_set_level(MP1,1);
    gpio_set_level(MP2,0);
    gpio_set_level(MP3,0);
    gpio_set_level(MP4,1);
    vTaskDelay(TMP / portTICK_PERIOD_MS); 
}
void passo_4()
{
    gpio_set_level(MP1,1);
    gpio_set_level(MP2,1);
    gpio_set_level(MP3,0);
    gpio_set_level(MP4,0);
    vTaskDelay(TMP / portTICK_PERIOD_MS); 
}
void passo_fim()
{
    gpio_set_level(MP1,0);
    gpio_set_level(MP2,0);
    gpio_set_level(MP3,0);
    gpio_set_level(MP4,0);
    vTaskDelay(TMP / portTICK_PERIOD_MS); 
}
void MP_horario(int passos)
{
    ESP_LOGI(TAG, "Rotacionando horário..");
    for (int i = passos * 4; i > 0; i--)
    {
        passo_4();
        passo_3();
        passo_2();
        passo_1();
    }
    passo_fim();
}
void MP_antihora(int passos)
{
    ESP_LOGI(TAG, "Rotacionando anti-horário..");
    for (int i = passos * 4; i > 0; i--)
    {   
        passo_1();
        passo_2();
        passo_3();
        passo_4();
    }
    passo_fim();
}

void rotacionar_DRV8825(bool horario, float angulo, int espera_us)
{
    int total_pulsos = (int)((PULSOS_POR_VOLTA * MICRO_PASSOS) * (angulo / 360.0));

    // Configura direção
    gpio_set_level(MP_DIR, horario);

    ESP_LOGI(TAG,"Girando %d passos (%0.2f graus) na direção %s", total_pulsos, angulo, horario ? "Horário" : "Anti-Horário");

    for (int i = 0; i < total_pulsos; i++)
    {
        gpio_set_level(MP_STEP, 1);
        esp_rom_delay_us(espera_us);  // Tempo em microssegundos para controlar velocidade
        gpio_set_level(MP_STEP, 0);
        esp_rom_delay_us(espera_us);
    }

    ESP_LOGI(TAG,"Rotação concluída!");
}


void rotacionar_MP(int horario, float graus) //64 passos por volta = 5,625 graus por passo
{
    if(DRV8825 == 0)
    {
        float passos = graus / 2.8125;
        ESP_LOGI(TAG, "Recebido %.1f graus, %d passos", graus, (int)passos);
        horario == 0 ? MP_antihora((int)passos) : MP_horario((int)passos);
        ESP_LOGI(TAG, "Rotação concluída");
    }
    else
    {
        rotacionar_DRV8825(horario, graus, 1000);

    }
}

void DRV_abrir(int passos)
{
    ESP_LOGI(TAG, "Rotacionando horário..");
    for (int i = passos * 4; i > 0; i--)
    {
        if(DRV8 & FF)break;
        vTaskDelay(TMP / portTICK_PERIOD_MS); 
        if(DRV7 & FF)break;
        vTaskDelay(TMP / portTICK_PERIOD_MS); 
        if(DRV6 & FF)break;
        vTaskDelay(TMP / portTICK_PERIOD_MS); 
        if(DRV5 & FF)break;
        vTaskDelay(TMP / portTICK_PERIOD_MS); 
        if(DRV4 & FF)break;
        vTaskDelay(TMP / portTICK_PERIOD_MS); 
        if(DRV3 & FF)break;
        vTaskDelay(TMP / portTICK_PERIOD_MS); 
        if(DRV2 & FF)break;
        vTaskDelay(TMP / portTICK_PERIOD_MS); 
        if(DRV1 & FF)break;
        vTaskDelay(TMP / portTICK_PERIOD_MS); 
    }
    DRV0;
}
void DRV_fechar(int passos)
{
    ESP_LOGI(TAG, "Rotacionando horário..");
    for (int i = passos * 4; i > 0; i--)
    {
        if(DRV1 & FI)break;
        vTaskDelay(TMP / portTICK_PERIOD_MS); 
        if(DRV2 & FI)break;
        vTaskDelay(TMP / portTICK_PERIOD_MS); 
        if(DRV3 & FI)break;
        vTaskDelay(TMP / portTICK_PERIOD_MS); 
        if(DRV4 & FI)break;
        vTaskDelay(TMP / portTICK_PERIOD_MS); 
        if(DRV5 & FI)break;
        vTaskDelay(TMP / portTICK_PERIOD_MS); 
        if(DRV6 & FI)break;
        vTaskDelay(TMP / portTICK_PERIOD_MS); 
        if(DRV7 & FI)break;
        vTaskDelay(TMP / portTICK_PERIOD_MS); 
        if(DRV8 & FI)break;
        vTaskDelay(TMP / portTICK_PERIOD_MS); 
    }
    DRV0;
}

void rotacionar_driver(int abrir, float graus, uint8_t saidas_atual) //64 passos por volta = 5,625 graus por passo
{
    float passos = graus / 2.8125;
    outputs = saidas_atual;
    ESP_LOGI(TAG, "Recebido %.1f graus, %d passos", graus, (int)passos);
    abrir == 0 ? DRV_fechar((int)passos) : DRV_abrir((int)passos);
    ESP_LOGI(TAG, "Rotação concluída");
}


void iniciar_MP(int modo)
{

    DRV8825 = modo;

    if (DRV8825 == 0){
        gpio_reset_pin(MP1);
        gpio_set_direction(MP1, GPIO_MODE_OUTPUT);
        gpio_set_level(MP1, 0);
        
        gpio_reset_pin(MP2);
        gpio_set_direction(MP2, GPIO_MODE_OUTPUT);
        gpio_set_level(MP2, 0);
        
        gpio_reset_pin(MP3);
        gpio_set_direction(MP3, GPIO_MODE_OUTPUT);
        gpio_set_level(MP3, 0);
        
        gpio_reset_pin(MP4);
        gpio_set_direction(MP4, GPIO_MODE_OUTPUT);
        gpio_set_level(MP4, 0);

        ESP_LOGI(TAG, "MP Configurado modo Driver ULN2003");
    }
    else 
    {
        gpio_reset_pin(MP_SLEEP);
        gpio_set_direction(MP_SLEEP, GPIO_MODE_OUTPUT);
        gpio_set_level(MP_SLEEP, 1);
        
        gpio_reset_pin(MP_STEP);
        gpio_set_direction(MP_STEP, GPIO_MODE_OUTPUT);
        gpio_set_level(MP_STEP, 0);
        
        gpio_reset_pin(MP_DIR);
        gpio_set_direction(MP_DIR, GPIO_MODE_OUTPUT);
        gpio_set_level(MP_DIR, 0);
        
        ESP_LOGI(TAG, "MP Configurado modo Driver DRV8825");
    }
}
void iniciar_driver(int Fim_de_curso_inicial, int Fim_de_curso_final)
{
    FC_INI = Fim_de_curso_inicial;
    FC_FIN = Fim_de_curso_final; 
}


