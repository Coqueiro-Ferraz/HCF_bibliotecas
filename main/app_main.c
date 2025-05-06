/*
*   Neste exemplo temos 4 devices registrados no Wegnology, usando o sensor DHT para publicar periodicamente no broker
*   Também subscreve ao tópico command, recebendo que vier pelo rótulo LED
*
*   Temos 2 modos, automatico e manual - automatico cria task para fazer um envio por batelada
*   Manual publica de modo normal os 2 atributos   
*
*   Autor: Hagaceef
*
*/

// Área de inclusão das bibliotecas
//-----------------------------------------------------------------------------------------------------------------------
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "HCF_SOFT.h"  
#include "HCF_DHT.h"
#include "driver/gpio.h"
#include "HCF_WNOLOGY.h"

#define TRIG_PIN 19  // Defina o pino TRIG
#define ECHO_PIN 21  // Defina o pino ECHO
#define DHT_PIN 23 //Defina o pino de dados o DHT /23


// Área das macros
//-----------------------------------------------------------------------------------------------------------------------
#define MODO 1 // Automatico
//#define MODO 0 // Manual

#define ESP 4 //1 2 3 4 -> Device

#define WIFI_SSID "coqueiro"
#define WIFI_PASS "amigos12"

//#define WIFI_SSID "GUEST"
//#define WIFI_PASS "cade204820"

#if (ESP==1)
    #define DEVICE_ID "65774aa82623fd911ab650c1" //ESP1
#elif (ESP==2)
    #define DEVICE_ID "6810f5b23c10b7b2e9e4e6d8" //ESP2
#elif (ESP==3)
    #define DEVICE_ID "6811023f30642df2ffeaa490" //ESP3
#else
    #define DEVICE_ID "6811025d30642df2ffeaa4da" //ESP4
#endif

#define W_ACCESS_KEY "76ac5ed2-ed18-4e96-9e02-d2dd572db083" //use a chave de acesso e a senha
#define W_PASSWORD "f52797619b7205bc2ac8d796d80fd0cb23f988e882cd0b82d575b26939f78c1c"

// Área de declaração de variáveis e protótipos de funções
//-----------------------------------------------------------------------------------------------------------------------

char *TAG = "HCF";
uint8_t entradas, saidas = 0; //variáveis de controle de entradas e saídas
char tecla = '-' ;
char escrever[40];
bool direcao = false;
int angulo = 0;
float temperatura = 0.0, umidade = 0.0;

// Funções e ramos auxiliares
//-----------------------------------------------------------------------------------------------------------------------

// Task para ler sensor e armazenar no buffer
void leitura_task(void *param) {
    //float temp, umid;
    while (true) {
        if (DHT_temp_umidade(&temperatura, &umidade)) {
            buffer_add(temperatura, umidade);  // Armazena temperatura, umidade e timestamp no buffer da lib
            ESP_LOGI(TAG, "Sensor lido e adicionado ao buffer: %.1f °C, %.1f %%", temperatura, umidade);
        } else {
            ESP_LOGW(TAG, "Falha ao ler sensor");
        }

        vTaskDelay(pdMS_TO_TICKS(10000)); // Aguarda 15s
    }
}

// Task para enviar dados armazenados a cada 1 minuto
void envio_task(void *param) {
    while (true) {
        enviar_buffer(); // Publica todos os dados armazenados
        check_wifi_reconnection(); // Verifica inatividade de Wi-Fi
        ESP_LOGI(TAG, "Lote enviado!");
        vTaskDelay(pdMS_TO_TICKS(60000)); // Aguarda 1 minuto
    }
}

// Função para receber dados e extrair o JSON do tópico command
/*
*   "name": "Ativando"
*   "payload": {"LED":"true"}
*   "time": "2025-05-05T16:15:22.263Z"
*/
void handler_led(const char *value) {
    if (strcmp(value, "true") == 0) {
        gpio_set_level(GPIO_NUM_2, 1);
    } else {
        gpio_set_level(GPIO_NUM_2, 0);
    }
}



// Programa Principal
//-----------------------------------------------------------------------------------------------------------------------

void app_main(void)
{
    /////////////////////////////////////////////////////////////////////////////////////   Programa principal

    /////////////////////////////////////////////////////////////////////////////////////   Inicializações de periféricos (manter assim)

    piscar_LED(3,2,100,100);
    iniciar_DHT(DHT_PIN);

    vTaskDelay(1000 / portTICK_PERIOD_MS); 

    iniciar_wnology_wifi(WIFI_SSID, WIFI_PASS, DEVICE_ID, W_ACCESS_KEY, W_PASSWORD); //Inicializa o MQTT
    wegnology_register_key_handler("LED", handler_led); //Regitra o atributo de subscrição do tópico command

    /////////////////////////////////////////////////////////////////////////////////////   Periféricos inicializados

#if (MODO == 1) 
    xTaskCreate(leitura_task, "leitura_task", 4096, NULL, 5, NULL);
    xTaskCreate(envio_task, "envio_task", 4096, NULL, 5, NULL);
    while(1){
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
#else
    while (1) {
        float temperatura=0, umidade=0;
        //char buffer[15];
        if(DHT_temp_umidade(&temperatura, &umidade))
        {
            // float temperatura = 25.0 + (rand() % 100) / 10.0f;  // valor simulado
            // mqtt_wegnology_send_float("Temperatura", temperatura); // envio direto
            // mqtt_wegnology_send_float("Umidade", umidade);

            char temp_str[16], umid_str[16];
            snprintf(temp_str, sizeof(temp_str), "%.2f", temperatura);
            snprintf(umid_str, sizeof(umid_str), "%.2f", umidade);

            const char *keys[] = { "Temperatura", "Umidade" }; //Chaves são os rótulos dos atributos do Wegnology
            const char *values[] = { temp_str, umid_str }; //Os valores são os valores a serem passados para os atributos

            mqtt_wegnology_publish_json(keys, values, 2, 1);


            printf("Temperatura: %.2f Umidade: %.2f\n", temperatura, umidade);
            ESP_LOGI(TAG, "Temperatura enviada: %.2f", temperatura);
        }
        else
        {
            printf("Erro sensor DHT\n");
        }
        vTaskDelay(pdMS_TO_TICKS(10000));  // publica a cada 10s
    }
#endif

    /////////////////////////////////////////////////////////////////////////////////////   Fim do ramo principal
    
}