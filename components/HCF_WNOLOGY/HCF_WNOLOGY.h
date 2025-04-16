#ifndef __HCF_WNOLOGY_H
    #define __HCF_WNOLOGY_H

//#include "mqtt_client.h"

// Retorno de status da publicação
typedef enum {
    MQTT_WEGNOLOGY_OK = 0,
    MQTT_WEGNOLOGY_ERROR_NOT_CONNECTED,
    MQTT_WEGNOLOGY_ERROR_NULL,
    MQTT_WEGNOLOGY_ERROR_JSON,
} mqtt_wegnology_status_t;

// Inicialização do módulo MQTT + Wi-Fi
void mqtt_wegnology_start(const char *ssid, const char *pass, const char *uri, const char *device_id, const char *user_name, const char *access_token);

// Definição dos tópicos de publicação e subscrição
void mqtt_wegnology_set_topics(const char *publish_topic, const char *subscribe_topic);

// Publicação de dados em JSON (apenas passa o conteúdo do corpo)
//void mqtt_wegnology_publish(const char *message_key, const char *message_value);

//void mqtt_wegnology_auto_configure(const char *device_id, const char *user_name, const char *access_token);
//void mqtt_wegnology_start(const char *ssid, const char *pass, const char *uri);

//Envio de atributo em formato float
void mqtt_wegnology_send_float(const char *key, float value);

// Versão estendida para múltiplas chaves/valores
mqtt_wegnology_status_t mqtt_wegnology_publish_json(const char **keys, const char **values, int count, int include_timestamp);

// Versão simplificada para chave/valor único
mqtt_wegnology_status_t mqtt_wegnology_publish(const char *key, const char *value);

// Callback customizado para processar dados recebidos via subscrição
void mqtt_wegnology_register_callback(void (*on_message)(const char *key, const char *value));

#endif