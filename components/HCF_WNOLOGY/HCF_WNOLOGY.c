#include "HCF_WNOLOGY.h"
//#include <stdio.h>
#include "HCF_WIFI.h"
#include "esp_log.h"
#include "cJSON.h"
//#include "esp_wifi.h"
#include "nvs_flash.h"
#include <time.h>
#include "esp_sntp.h"
#include <string.h>
#include "mqtt_client.h"
#include "cJSON.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

/*
Exemplo de configuração
#define W_DEVICE_ID "65774aa82623fd911ab650c1" //Use o DeviceID no Wegnology  
#define W_ACCESS_KEY "76ac5ed2-ed18-4e96-9e02-d2dd572db083" //use a chave de acesso e a senha
#define W_PASSWORD "f52797619b7205bc2ac8d796d80fd0cb23f988e882cd0b82d575b26939f78c1c"
#define W_TOPICO_PUBLICAR "wnology/65774aa82623fd911ab650c1/state" //esse número no meio do tópico deve ser mudado pelo ID do seu device Wegnology
#define W_TOPICO_SUBSCREVER "wnology/65774aa82623fd911ab650c1/command" // aqui também
#define W_BROKER "mqtt://broker.app.wnology.io:1883"
#define SSID "coqueiro"
#define PASSWORD "amigos12"
*/

static const char *TAG = "HCF_WNOLOGY";

static esp_mqtt_client_handle_t mqtt_client = NULL;
static char dev_id[64];
static char publish_topic[128];
static char subscribe_topic[128];
static char mqtt_username[64];
static char mqtt_password[128];

static bool mqtt_connected = false;

static void (*user_callback)(const char *, const char *) = NULL;

static QueueHandle_t mqtt_msg_queue;

typedef struct {
    char key[64];
    float value;
} mqtt_message_t;

// Callback do evento MQTT
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {
    esp_mqtt_event_handle_t event = event_data;

    switch ((esp_mqtt_event_id_t)event_id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
            esp_mqtt_client_subscribe(mqtt_client, subscribe_topic, 0);
            mqtt_connected = true;
 
            break;

        /*case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT_EVENT_DATA RECEIVED");
            printf("TOPIC=%.*s\n", event->topic_len, event->topic);
            printf("DATA=%.*s\n", event->data_len, event->data);
            break;*/
        case MQTT_EVENT_DATA:
            cJSON *json = cJSON_ParseWithLength(event->data, event->data_len);
            if (json) {
                cJSON *item = json->child;
                while (item) {
                    if (user_callback) user_callback(item->string, item->valuestring);
                    item = item->next;
                }
                cJSON_Delete(json);
            }
            ESP_LOGI(TAG, "Mensagem recebida:");
            ESP_LOGI(TAG, "TOPIC=%.*s", event->topic_len, event->topic);
            ESP_LOGI(TAG, "DATA=%.*s", event->data_len, event->data);
    
    /*// Aqui você chama a função de callback definida pelo usuário
    if (internal_rx_callback) {
        char topic[event->topic_len + 1];
        char data[event->data_len + 1];
        memcpy(topic, event->topic, event->topic_len);
        topic[event->topic_len] = '\0';
        memcpy(data, event->data, event->data_len);
        data[event->data_len] = '\0';
        internal_rx_callback(topic, data);
    }*/









            break;
        case MQTT_EVENT_DISCONNECTED:
            mqtt_connected = false;
            break;
        default:
            break;
    }
}

void mqtt_wegnology_auto_configure(const char *device_id, const char *user_name, const char *access_token) {
   // dev_id = device_id;
    snprintf(publish_topic, sizeof(publish_topic), "wnology/%s/state", device_id);
    snprintf(subscribe_topic, sizeof(subscribe_topic), "wnology/%s/command", device_id);
    snprintf(mqtt_username, sizeof(mqtt_username), "%s", user_name);
    snprintf(mqtt_password, sizeof(mqtt_password), "%s", access_token);
}

static void mqtt_publish_task(void *param) {
    mqtt_message_t msg;
    while (1) {
        if (xQueueReceive(mqtt_msg_queue, &msg, portMAX_DELAY)) {
            cJSON *root = cJSON_CreateObject();
            cJSON *data = cJSON_CreateObject();
            cJSON_AddNumberToObject(data, msg.key, msg.value);
            cJSON_AddItemToObject(root, "data", data);

            // Adiciona timestamp automático (poderia vir de um RTC ou NTP futuramente) ou receber da própria wegnology
            cJSON_AddStringToObject(root, "time", "2025-04-16T00:00:00.000Z");

            char *json_str = cJSON_PrintUnformatted(root);
            esp_mqtt_client_publish(mqtt_client, publish_topic, json_str, 0, 1, 0);
            ESP_LOGI(TAG, "Publicado: %s", json_str);
            cJSON_Delete(root);
            free(json_str);
        }
    }
}

// Inicializa Wi-Fi e MQTT
void mqtt_wegnology_start(const char *ssid, const char *pass, const char *uri, const char *device_id, const char *user_name, const char *access_token) {

    nvs_flash_init();
    //tcpip_adapter_init();
    wifi_init();
    wifi_connect_sta(ssid, pass, 10000);

    snprintf(dev_id, sizeof(dev_id), "%s", device_id);
    snprintf(publish_topic, sizeof(publish_topic), "wnology/%s/state", device_id);
    snprintf(subscribe_topic, sizeof(subscribe_topic), "wnology/%s/command", device_id);
    snprintf(mqtt_username, sizeof(mqtt_username), "%s", user_name);
    snprintf(mqtt_password, sizeof(mqtt_password), "%s", access_token);

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = uri,
        .credentials.set_null_client_id = false,  
        .credentials.client_id = dev_id,
        .credentials.username = mqtt_username,
        .credentials.authentication.password = mqtt_password,
    };
    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);

    mqtt_msg_queue = xQueueCreate(10, sizeof(mqtt_message_t));
    xTaskCreate(mqtt_publish_task, "mqtt_pub_task", 4096, NULL, 5, NULL);
}


/*void mqtt_wegnology_start(const char *ssid, const char *pass, const char *uri, const char *dev_id, const char *wkey, const char *wpwd) {
    nvs_flash_init();
    //tcpip_adapter_init();
    wifi_init();
    wifi_connect_sta(ssid, pass, 10000);


    // Configuração e conexão Wi-Fi omitida aqui por brevidade...

    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = uri,
        .credentials.set_null_client_id = false,  
        .credentials.client_id = dev_id,
        .credentials.username = wkey,
        .credentials.authentication.password = wpwd,
    };
    client = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}*/

void mqtt_wegnology_send_float(const char *key, float value){
    mqtt_message_t msg;
    snprintf(msg.key, sizeof(msg.key), "%s", key);
    msg.value = value;
    xQueueSend(mqtt_msg_queue, &msg, portMAX_DELAY);
}



void mqtt_wegnology_stop() {
    if (mqtt_client) {
        esp_mqtt_client_stop(mqtt_client);
        esp_mqtt_client_destroy(mqtt_client);
        mqtt_client = NULL;
    }
}

void mqtt_wegnology_set_topics(const char *pub, const char *sub) {
    strncpy(publish_topic, pub, sizeof(publish_topic));
    strncpy(subscribe_topic, sub, sizeof(subscribe_topic));
}

/*void mqtt_wegnology_publish(const char *key, const char *value) {
    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, key, value);
    char *json_str = cJSON_PrintUnformatted(root);
    esp_mqtt_client_publish(client, publish_topic, json_str, 0, 1, 0);
    free(json_str);
    cJSON_Delete(root);
}*/

// Publica um único par chave/valor
mqtt_wegnology_status_t mqtt_wegnology_publish(const char *key, const char *value) {
    if (!mqtt_client || !key || !value) return MQTT_WEGNOLOGY_ERROR_NULL;
    if (!mqtt_connected) return MQTT_WEGNOLOGY_ERROR_NOT_CONNECTED;

    cJSON *root = cJSON_CreateObject();
    if (!root) return MQTT_WEGNOLOGY_ERROR_JSON;

    cJSON_AddStringToObject(root, key, value);
    char *json_str = cJSON_PrintUnformatted(root);
    mqtt_wegnology_status_t status = MQTT_WEGNOLOGY_OK;

    if (esp_mqtt_client_publish(mqtt_client, publish_topic, json_str, 0, 1, 0) < 0) {
        status = MQTT_WEGNOLOGY_ERROR_JSON;
    }

    free(json_str);
    cJSON_Delete(root);
    return status;
}

// Publica múltiplos pares chave/valor + timestamp opcional
mqtt_wegnology_status_t mqtt_wegnology_publish_json(const char **keys, const char **values, int count, int include_timestamp) {
    if (!mqtt_client || !keys || !values || count <= 0) return MQTT_WEGNOLOGY_ERROR_NULL;
    if (!mqtt_connected) return MQTT_WEGNOLOGY_ERROR_NOT_CONNECTED;

    cJSON *root = cJSON_CreateObject();
    if (!root) return MQTT_WEGNOLOGY_ERROR_JSON;

    for (int i = 0; i < count; i++) {
        cJSON_AddStringToObject(root, keys[i], values[i]);
    }

    if (include_timestamp) {
        time_t now;
        time(&now);
        cJSON_AddNumberToObject(root, "timestamp", (double)now);
    }

    char *json_str = cJSON_PrintUnformatted(root);
    mqtt_wegnology_status_t status = MQTT_WEGNOLOGY_OK;

    if (esp_mqtt_client_publish(mqtt_client, publish_topic, json_str, 0, 1, 0) < 0) {
        status = MQTT_WEGNOLOGY_ERROR_JSON;
    }

    free(json_str);
    cJSON_Delete(root);
    return status;
}

void mqtt_wegnology_register_callback(void (*callback)(const char *key, const char *value)) {
    user_callback = callback;
}


const char *strLED = "LED\":"; 
const char *subtopico_temp = "{\"data\": {\"Temperatura\": ";

/*void mensagem(esp_mqtt_client_handle_t cliente) 
{
   if(strstr(Inform, strLED)) // aqui estou perguntando se o que foi publicado no tópico command está relacionado a minha TAG LED
   {//caso afirmativo
        
       if(strstr(Inform, "true")) // aqui pergunto se a TAG LED recebeu o valor "true"
       {//se "LED":"true"
           ledstatus = 1;//inverte LED embarcado
            if(ledstatus==1)//se o LED embarcado está ligado
            {
                printf("LED ligado\n");
                // publico no tópico state {"data":{"est_esp":"link da imagem"}}
                esp_mqtt_client_publish(cliente, W_TOPICO_PUBLICAR, 
               // "{\"data\": {\"est_esp\": \"https://files.wnology.io/65774aa82623fd911ab650c1/imagens/esp%20led.png\" }}", 0, 0, 0);
                "{\"data\": {\"est_esp\": \"https://files.wnology.io/65774aa72623fd911ab650bf/imagens/esp%20led.png\" }}", 0, 0, 0);
            }
            else
            {
                printf("LED desligado\n");
                esp_mqtt_client_publish(cliente, W_TOPICO_PUBLICAR, 
                //"{\"data\": {\"est_esp\": \"https://files.wnology.io/65774aa82623fd911ab650c1/imagens/esp%20ligado.png\" }}", 0, 0, 0);
                "{\"data\": {\"est_esp\": \"https://files.wnology.io/65774aa72623fd911ab650bf/imagens/esp%20ligado.png\" }}", 0, 0, 0);
            }
       }
       else
       {//se "LED":"false"
         //   temp_val = rand() % 100;  // valor aleatório entre 0 e 100 para simular a temperatura
         //   sprintf(&mensa[0],"%s %d }}",string_temp,temp_val);//isso tudo aqui é só pra mostrar no console o que estou enviando pro broker
         //   ESP_LOGI(TAG, "%s", &mensa[0]);//vai aparecer de verde no console: MQTT_EXAMPLE: {"data": {Temperatura": 25 }}
         //   esp_mqtt_client_publish(cliente, W_TOPICO_PUBLICAR, &mensa[0], 0, 0, 0); // Aqui é onde está fazendo a publicação no broker efetivamente
        
       
       }
        
       // gpio_set_level(TEC_SH_LD,ledstatus);//aqui é onde realmente acende ou apaga o LED embarcado
    
    }

}*/


/*static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}*/

/*static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {

    case MQTT_EVENT_CONNECTED:        
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        msg_id = esp_mqtt_client_publish(client, W_TOPICO_PUBLICAR,mensa, 0, 0, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        ESP_LOGI(TAG, "%s", mensa);       
        msg_id = esp_mqtt_client_subscribe(client, W_TOPICO_SUBSCREVER, 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;

    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        Inform = event->data;
        mensagem(client); 
        break;

    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
        }
        break;

    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}*/


/*static void mqtt_app_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = W_BROKER,
        .credentials.set_null_client_id = false,  
        .credentials.client_id = W_DEVICE_ID,
        .credentials.username = W_ACCESS_KEY,
        .credentials.authentication.password = W_PASSWORD,
        
    };
    cliente = esp_mqtt_client_init(&mqtt_cfg);
    esp_mqtt_client_register_event(cliente, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(cliente);
}*/

