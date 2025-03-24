//////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                       _              //
//               _    _       _      _        _     _   _   _    _   _   _        _   _  _   _          //
//           |  | |  |_| |\| |_| |\ |_|   |\ |_|   |_| |_| | |  |   |_| |_| |\/| |_| |  |_| | |         //    
//         |_|  |_|  |\  | | | | |/ | |   |/ | |   |   |\  |_|  |_| |\  | | |  | | | |_ | | |_|         //
//                                                                                       /              //
//////////////////////////////////////////////////////////////////////////////////////////////////////////

/*
*   Programa básico para controle da placa durante a Jornada da Programação 1
*   Permite o controle das entradas e saídas digitais, entradas analógicas, display LCD e teclado. 
*   Cada biblioteca pode ser consultada na respectiva pasta em componentes
*   Existem algumas imagens e outros documentos na pasta Recursos
*
*   Autor: Hagaceef
*
*/

// Área de inclusão das bibliotecas
//-----------------------------------------------------------------------------------------------------------------------
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "HCF_IOTEC.h"   // Vai se tornar HCF_IOTEC
#include "HCF_LCD.h" // Vai se tornar HCF_LCD
#include "HCF_ADC.h"   // Vai se tornar HCF_ADC
#include "HCF_MP.h"   // Vai se tornar HCF_MP
// Incluir HCF_IOT HCF_BT HCF_DHT HCF_ULTRA HCF_RFID HCF_ZMPT HCF_ACS HCF_SERVO HCF_OLED HCF_CAM HCF_SD HCF_LORA HCF_DIGITAL

#include "HCF_ULTRA.h"
#include "HCF_DHT.h"


#define TRIG_PIN 19  // Defina o pino TRIG
#define ECHO_PIN 21  // Defina o pino ECHO
#define DHT_PIN 23 //Defina o pino de dados o DHT

// Área das macros
//-----------------------------------------------------------------------------------------------------------------------



#define IN(x) (entradas>>x)&1

// Área de declaração de variáveis e protótipos de funções
//-----------------------------------------------------------------------------------------------------------------------

static const char *TAG = "Placa";
static uint8_t entradas, saidas = 0; //variáveis de controle de entradas e saídas
static char tecla = '-' ;
char escrever[40];
bool direcao = false;
int angulo = 0;
float temperatura = 0.0, umidade = 0.0;

// Funções e ramos auxiliares
//-----------------------------------------------------------------------------------------------------------------------




// Programa Principal
//-----------------------------------------------------------------------------------------------------------------------

void app_main(void)
{
    /////////////////////////////////////////////////////////////////////////////////////   Programa principal
    escrever[39] = '\0';

    // a seguir, apenas informações de console, aquelas notas verdes no início da execução
    ESP_LOGI(TAG, "Iniciando...");
    ESP_LOGI(TAG, "Versão do IDF: %s", esp_get_idf_version());

    /////////////////////////////////////////////////////////////////////////////////////   Inicializações de periféricos (manter assim)
    
    // inicializar os IOs e teclado da placa
    iniciar_iotec();      
    entradas = io_le_escreve(saidas); // Limpa as saídas e lê o estado das entradas


    iniciar_ultrassonico(TRIG_PIN,ECHO_PIN);  // Inicializa o sensor

    iniciar_MP(1);
    // inicializar o display LCD 
    iniciar_lcd();
    escreve_lcd(1,0,"    Jornada 1   ");
    escreve_lcd(2,0," Programa Basico");
    
    // Inicializar o componente de leitura de entrada analógica
    esp_err_t init_result = iniciar_adc_CHX(0);
    if (init_result != ESP_OK) {
        ESP_LOGE("MAIN", "Erro ao inicializar o componente ADC personalizado");
    }

    //iniciar_DHT(DHT_PIN);  // Inicializa o sensor
    //delay inicial
    vTaskDelay(1000 / portTICK_PERIOD_MS); 
    limpar_lcd();


    /////////////////////////////////////////////////////////////////////////////////////   Periféricos inicializados
   // xTaskCreate(&DHT_task, "DHT_task", 2048, NULL, 3, NULL); 


    /////////////////////////////////////////////////////////////////////////////////////   Início do ramo principal                    
    while (1)                                                                                                                         
    {                                                                                                                                 
        //_______________________________________________________________________________________________________________________________________________________ //
        //-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - -  -  -  -  -  -  -  -  -  -  Escreva seu código aqui!!! //
        tecla = le_teclado();     
        if(tecla > '0' && tecla <= '8')
        {
            saidas = 1 << (tecla - '0' - 1);
        }            
        else if (tecla == '9')
        {
            saidas = 0xFF;
        }                                  
        else if (tecla == '0')
        {
            saidas = 0x00;
        }                                                                                                      
        
        entradas = io_le_escreve(saidas);


        /*if (DHT_temp_umidade(&temperatura, &umidade)) {
            ESP_LOGI(TAG,"Temperatura: %.1f°C, Umidade: %.1f%%", temperatura, umidade);
            sprintf(escrever,"Temperatura:%.1f°",temperatura);
            escreve_lcd(1,0,escrever);
            sprintf(escrever,"Umidade: %.1f%%  ",umidade);
            escreve_lcd(2,0,escrever);

        } else {
            ESP_LOGW(TAG,"Falha na leitura do sensor DHT22");
            escreve_lcd(1,0,"Erro de leitura ");
            escreve_lcd(2,0,"                ");
        }
        if(temperatura >= 27.0) saidas = 0x01;
        else saidas = 0x00;
        */
      //  sprintf(escrever, "INPUTS:%d%d%d%d%d%d%d%d", IN(7),IN(6),IN(5),IN(4),IN(3),IN(2),IN(1),IN(0));
      //  escreve_lcd(1,0,escrever);

     //   sprintf(escrever, "%c", tecla);
      //  escreve_lcd(2,14,escrever);

        
       /* float distancia = medir_distancia();  // Obtém a distância
        ESP_LOGI(TAG, "Distância: %.2f cm", distancia);  // Imprime a distância
        sprintf(escrever, "Dist: %.2f cm  ", distancia);
        escreve_lcd(1,0,escrever);*/
        
        
        angulo = angulo + (direcao==true?-18:18);
        if(angulo>=160){direcao = true;}
        else if(angulo<=20){direcao = false;}
        rotacionar_DRV8825(direcao,180,1000);
        sprintf(escrever, "Ang: %d hora:%d  ", angulo, direcao);
        escreve_lcd(2,0,escrever);

        //vTaskDelay(pdMS_TO_TICKS(1000));  // Aguarda 1 segundo antes de repetir

        //-  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  -  - -  -  -  -  -  -  -  -  -  -  Escreva seu só até aqui!!! //
        //________________________________________________________________________________________________________________________________________________________//
        vTaskDelay(100 / portTICK_PERIOD_MS);    // delay mínimo obrigatório, se retirar, pode causar reset do ESP
    }
    
    // caso erro no programa, desliga o módulo ADC
    adc_limpar();

    /////////////////////////////////////////////////////////////////////////////////////   Fim do ramo principal
    
}
