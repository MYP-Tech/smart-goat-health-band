/*
 * DS18B20 — 1-Wire bit-banged driver
 *
 * For production firmware, swap this for a maintained component, e.g.
 * `idf-component-manager add esp_idf_lib/onewire`. This implementation is
 * intentionally minimal and self-contained for the MVP.
 *
 * Wiring:
 *   DATA → GPIO 4
 *   VCC  → 3V3
 *   GND  → GND
 *   4.7kΩ pullup from DATA to 3V3 (REQUIRED)
 */
#include "ds18b20.h"
#include "pinmap.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "rom/ets_sys.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "ds18b20";

#define OW_PIN  DS18B20_GPIO

static inline void ow_release(void)  { gpio_set_direction(OW_PIN, GPIO_MODE_INPUT);  }
static inline void ow_pull_low(void) { gpio_set_direction(OW_PIN, GPIO_MODE_OUTPUT); gpio_set_level(OW_PIN, 0); }
static inline int  ow_read(void)     { return gpio_get_level(OW_PIN); }

// 1-Wire reset pulse + presence detect. Returns true if DS18B20 is on the bus.
static bool ow_reset(void)
{
    ow_pull_low();
    ets_delay_us(480);
    ow_release();
    ets_delay_us(70);
    bool present = (ow_read() == 0);
    ets_delay_us(410);
    return present;
}

static void ow_write_bit(int bit)
{
    if (bit) {
        ow_pull_low();
        ets_delay_us(6);
        ow_release();
        ets_delay_us(64);
    } else {
        ow_pull_low();
        ets_delay_us(60);
        ow_release();
        ets_delay_us(10);
    }
}

static int ow_read_bit(void)
{
    ow_pull_low();
    ets_delay_us(6);
    ow_release();
    ets_delay_us(9);
    int b = ow_read();
    ets_delay_us(55);
    return b;
}

static void ow_write_byte(uint8_t b)
{
    for (int i = 0; i < 8; i++) {
        ow_write_bit(b & 0x01);
        b >>= 1;
    }
}

static uint8_t ow_read_byte(void)
{
    uint8_t b = 0;
    for (int i = 0; i < 8; i++) {
        b >>= 1;
        if (ow_read_bit()) b |= 0x80;
    }
    return b;
}

esp_err_t ds18b20_init(void)
{
    gpio_config_t cfg = {
        .pin_bit_mask = (1ULL << OW_PIN),
        .mode         = GPIO_MODE_INPUT,    // start in high-impedance
        .pull_up_en   = GPIO_PULLUP_DISABLE, // external 4.7k pullup
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    ESP_ERROR_CHECK(gpio_config(&cfg));

    if (!ow_reset()) {
        ESP_LOGW(TAG, "no presence pulse — check wiring and 4.7kΩ pullup");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG, "DS18B20 initialized");
    return ESP_OK;
}

float ds18b20_read_temp(void)
{
    if (!ow_reset()) return -127.0f;

    ow_write_byte(0xCC); // SKIP ROM
    ow_write_byte(0x44); // CONVERT T

    // 12-bit conversion takes up to 750ms. Don't busy-wait; yield to scheduler.
    vTaskDelay(pdMS_TO_TICKS(800));

    if (!ow_reset()) return -127.0f;
    ow_write_byte(0xCC); // SKIP ROM
    ow_write_byte(0xBE); // READ SCRATCHPAD

    uint8_t lsb = ow_read_byte();
    uint8_t msb = ow_read_byte();

    int16_t raw = (msb << 8) | lsb;
    return raw * 0.0625f;   // 12-bit resolution: 1 LSB = 0.0625 °C
}
