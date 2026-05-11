/*
 * GoatBand MVP-1 — main entry point
 * Pure ESP-IDF · NodeMCU-32S · no Arduino
 *
 * Architecture:
 *   - app_main initializes peripherals and spawns 3 FreeRTOS tasks:
 *       1. motion_task     — samples MPU-6050 at 20 Hz, accumulates
 *       2. telemetry_task  — every 30s, reads temp + battery, computes score, publishes
 *       3. ble_task        — runs Bluedroid stack, exposes characteristic
 *   - Tasks communicate via shared atomic state guarded by a mutex
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/i2c.h"
#include "esp_system.h"

#include "pinmap.h"
#include "mpu6050.h"
#include "ds18b20.h"
#include "battery.h"
#include "ble_service.h"
#include "activity_score.h"

static const char *TAG = "goatband";

// Shared state between motion task and telemetry task
static SemaphoreHandle_t state_mutex;
static float motion_accum  = 0.0f;
static uint32_t motion_count = 0;

static void i2c_master_init(void)
{
    i2c_config_t conf = {
        .mode             = I2C_MODE_MASTER,
        .sda_io_num       = I2C_MASTER_SDA_IO,
        .scl_io_num       = I2C_MASTER_SCL_IO,
        .sda_pullup_en    = GPIO_PULLUP_ENABLE,
        .scl_pullup_en    = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_MASTER_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_MASTER_NUM, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0));
}

// 20 Hz motion sampling loop. Subtracts gravity from acceleration magnitude,
// accumulates into a window. Telemetry task drains it every 30 seconds.
static void motion_task(void *arg)
{
    const TickType_t period = pdMS_TO_TICKS(1000 / MOTION_SAMPLE_HZ);
    mpu6050_data_t data;

    while (1) {
        if (mpu6050_read(&data) == ESP_OK) {
            float mag = sqrtf(data.ax * data.ax +
                              data.ay * data.ay +
                              data.az * data.az);
            float motion = fabsf(mag - 9.81f);   // strip gravity component

            if (xSemaphoreTake(state_mutex, portMAX_DELAY) == pdTRUE) {
                motion_accum += motion;
                motion_count++;
                xSemaphoreGive(state_mutex);
            }
        }
        vTaskDelay(period);
    }
}

// Every 30 seconds: read temp, read battery, compute score, publish via BLE.
static void telemetry_task(void *arg)
{
    const TickType_t period = pdMS_TO_TICKS(WINDOW_DURATION_MS);
    char json[256];

    while (1) {
        vTaskDelay(period);

        // Snapshot and reset motion accumulator
        float avg_motion = 0.0f;
        if (xSemaphoreTake(state_mutex, portMAX_DELAY) == pdTRUE) {
            avg_motion = motion_count > 0 ? motion_accum / motion_count : 0.0f;
            motion_accum = 0.0f;
            motion_count = 0;
            xSemaphoreGive(state_mutex);
        }

        float temp_c  = ds18b20_read_temp();
        float batt_v  = battery_read_voltage();
        int   score   = activity_score_compute(avg_motion);

        int n = snprintf(json, sizeof(json),
            "{\"t\":%lu,\"motion\":%.3f,\"temp\":%.2f,\"batt\":%.2f,\"score\":%d}",
            (unsigned long)(esp_log_timestamp() / 1000),
            avg_motion, temp_c, batt_v, score);

        ESP_LOGI(TAG, "%s", json);

        if (n > 0 && n < (int)sizeof(json)) {
            ble_service_notify((const uint8_t *)json, n);
        }
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "GoatBand MVP-1 starting");

    // Initialize NVS — needed for BLE bonding state and per-goat baselines
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Bring up peripherals
    i2c_master_init();
    ESP_ERROR_CHECK(mpu6050_init());
    ESP_ERROR_CHECK(ds18b20_init());
    ESP_ERROR_CHECK(battery_init());
    ESP_ERROR_CHECK(ble_service_init());

    state_mutex = xSemaphoreCreateMutex();
    configASSERT(state_mutex);

    // Spawn worker tasks. Motion sampling is timing-critical → higher priority.
    xTaskCreatePinnedToCore(motion_task,    "motion",    4096, NULL, 6, NULL, 1);
    xTaskCreatePinnedToCore(telemetry_task, "telemetry", 4096, NULL, 4, NULL, 0);

    ESP_LOGI(TAG, "All tasks running. Advertising as %s", BLE_DEVICE_NAME);
}
