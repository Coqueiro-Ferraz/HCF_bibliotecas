/*#include "bt_server.h"
#include "esp_log.h"
#include "esp_spp_api.h"
#include "esp_bt.h"
#include "esp_bt_main.h"

#define SPP_SERVER_NAME "ESP32_BT_SERVER"

static const char *TAG = "BT_SERVER";
static uint32_t bt_server_connection_handle = 0;
static bt_server_recv_cb_t user_recv_callback = NULL;

static void spp_event_handler(esp_spp_cb_event_t event, esp_spp_cb_param_t *param) {
    switch (event) {
        case ESP_SPP_INIT_EVT:
            ESP_LOGI(TAG, "SPP inicializado");
            esp_spp_start_srv(ESP_SPP_SEC_NONE, ESP_SPP_ROLE_SLAVE, 0, SPP_SERVER_NAME);
            break;

        case ESP_SPP_START_EVT:
            ESP_LOGI(TAG, "Servidor SPP iniciado, aguardando conexão...");
            break;

        case ESP_SPP_SRV_OPEN_EVT:
            ESP_LOGI(TAG, "Cliente conectado");
            bt_server_connection_handle = param->srv_open.handle;
            break;

        case ESP_SPP_CLOSE_EVT:
            ESP_LOGI(TAG, "Conexão encerrada");
            bt_server_connection_handle = 0;
            break;

        case ESP_SPP_DATA_IND_EVT:
            if (user_recv_callback && param->data_ind.len > 0) {
                user_recv_callback((char)param->data_ind.data[0]);
            }
            break;

        default:
            break;
    }
}

esp_err_t bt_server_init(bt_server_recv_cb_t recv_cb) {
    user_recv_callback = recv_cb;

    esp_err_t ret = ESP_OK;

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();

    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) return ret;

    ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT);
    if (ret) return ret;

    ret = esp_bluedroid_init();
    if (ret) return ret;

    ret = esp_bluedroid_enable();
    if (ret) return ret;

    ret = esp_spp_register_callback(spp_event_handler);
    if (ret) return ret;

    ret = esp_spp_init(ESP_SPP_MODE_CB);
    return ret;
}

esp_err_t bt_server_send_char(char data) {
    if (bt_server_connection_handle != 0) {
        return esp_spp_write(bt_server_connection_handle, 1, (uint8_t *)&data);
    } else {
        return ESP_FAIL;
    }
}*/