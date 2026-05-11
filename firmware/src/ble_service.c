/*
 * BLE GATT server — Bluedroid stack
 *
 * Exposes one custom service with one characteristic (NOTIFY + READ).
 * UUID: 12345678-1234-5678-1234-56789ABCDEF0  (service)
 *       12345678-1234-5678-1234-56789ABCDEF1  (characteristic)
 *
 * Pair from phone using nRF Connect (Nordic). Subscribe to the characteristic
 * to see JSON telemetry stream every 30 seconds.
 */
#include "ble_service.h"
#include "pinmap.h"
#include <string.h>

#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_log.h"

static const char *TAG = "ble";

#define APP_ID                      0x55
#define SVC_INST_ID                 0
#define CHAR_VAL_MAX_LEN            240

// 128-bit UUIDs (little-endian byte order in the array below)
static const uint8_t SVC_UUID[16]  = {
    0xF0,0xCD,0xAB,0x89,0x67,0x45, 0x34,0x12, 0x78,0x56, 0x34,0x12, 0x78,0x56,0x34,0x12 };
static const uint8_t CHAR_UUID[16] = {
    0xF1,0xCD,0xAB,0x89,0x67,0x45, 0x34,0x12, 0x78,0x56, 0x34,0x12, 0x78,0x56,0x34,0x12 };

static uint16_t g_conn_id   = 0xFFFF;
static uint16_t g_char_handle = 0;
static esp_gatt_if_t g_gatts_if = ESP_GATT_IF_NONE;

static esp_ble_adv_data_t adv_data = {
    .set_scan_rsp        = false,
    .include_name        = true,
    .include_txpower     = false,
    .min_interval        = 0x0006,
    .max_interval        = 0x0010,
    .appearance          = 0x00,
    .manufacturer_len    = 0,
    .p_manufacturer_data = NULL,
    .service_data_len    = 0,
    .p_service_data      = NULL,
    .service_uuid_len    = sizeof(SVC_UUID),
    .p_service_uuid      = (uint8_t *)SVC_UUID,
    .flag                = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

static esp_ble_adv_params_t adv_params = {
    .adv_int_min       = 0x20,
    .adv_int_max       = 0x40,
    .adv_type          = ADV_TYPE_IND,
    .own_addr_type     = BLE_ADDR_TYPE_PUBLIC,
    .channel_map       = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

static void gap_cb(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event) {
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        esp_ble_gap_start_advertising(&adv_params);
        break;
    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
        ESP_LOGI(TAG, "advertising started");
        break;
    default:
        break;
    }
}

static void gatts_cb(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if,
                     esp_ble_gatts_cb_param_t *p)
{
    switch (event) {
    case ESP_GATTS_REG_EVT: {
        g_gatts_if = gatts_if;
        esp_ble_gap_set_device_name(BLE_DEVICE_NAME);

        esp_gatt_srvc_id_t svc = {
            .is_primary = true,
            .id = {
                .inst_id = SVC_INST_ID,
                .uuid = { .len = ESP_UUID_LEN_128, .uuid = {0} }
            }
        };
        memcpy(svc.id.uuid.uuid.uuid128, SVC_UUID, 16);
        esp_ble_gatts_create_service(gatts_if, &svc, 4);
        break;
    }
    case ESP_GATTS_CREATE_EVT: {
        esp_ble_gatts_start_service(p->create.service_handle);
        esp_bt_uuid_t char_uuid = { .len = ESP_UUID_LEN_128, .uuid = {0} };
        memcpy(char_uuid.uuid.uuid128, CHAR_UUID, 16);

        esp_attr_value_t initial = {
            .attr_max_len = CHAR_VAL_MAX_LEN,
            .attr_len     = 0,
            .attr_value   = NULL,
        };
        esp_ble_gatts_add_char(
            p->create.service_handle, &char_uuid,
            ESP_GATT_PERM_READ,
            ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY,
            &initial, NULL);

        esp_ble_gap_config_adv_data(&adv_data);
        break;
    }
    case ESP_GATTS_ADD_CHAR_EVT:
        g_char_handle = p->add_char.attr_handle;
        ESP_LOGI(TAG, "characteristic handle = %d", g_char_handle);
        break;
    case ESP_GATTS_CONNECT_EVT:
        g_conn_id = p->connect.conn_id;
        ESP_LOGI(TAG, "client connected");
        break;
    case ESP_GATTS_DISCONNECT_EVT:
        g_conn_id = 0xFFFF;
        ESP_LOGI(TAG, "client disconnected, restart advertising");
        esp_ble_gap_start_advertising(&adv_params);
        break;
    default:
        break;
    }
}

esp_err_t ble_service_init(void)
{
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BLE));
    ESP_ERROR_CHECK(esp_bluedroid_init());
    ESP_ERROR_CHECK(esp_bluedroid_enable());

    ESP_ERROR_CHECK(esp_ble_gap_register_callback(gap_cb));
    ESP_ERROR_CHECK(esp_ble_gatts_register_callback(gatts_cb));
    ESP_ERROR_CHECK(esp_ble_gatts_app_register(APP_ID));

    return ESP_OK;
}

void ble_service_notify(const uint8_t *data, size_t len)
{
    if (g_conn_id == 0xFFFF || g_char_handle == 0) return;
    if (len > CHAR_VAL_MAX_LEN) len = CHAR_VAL_MAX_LEN;

    esp_ble_gatts_send_indicate(g_gatts_if, g_conn_id, g_char_handle,
                                 len, (uint8_t *)data, false);
}
