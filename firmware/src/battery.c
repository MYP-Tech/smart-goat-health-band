/*
 * Battery voltage via 100k/100k divider on GPIO 35 (ADC1_CH7).
 * Uses ESP-IDF v5 ADC oneshot driver (the legacy adc1_get_raw is deprecated).
 */
#include "battery.h"
#include "pinmap.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_log.h"

static const char *TAG = "battery";

static adc_oneshot_unit_handle_t   adc_handle;
static adc_cali_handle_t           cali_handle = NULL;
static bool                        cali_enabled = false;

esp_err_t battery_init(void)
{
    adc_oneshot_unit_init_cfg_t init_cfg = {
        .unit_id = BATT_ADC_UNIT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_cfg, &adc_handle));

    adc_oneshot_chan_cfg_t chan_cfg = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten    = ADC_ATTEN_DB_12,    // ~150mV–3.3V input range
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, BATT_ADC_CHANNEL, &chan_cfg));

    // Try curve-fitting calibration scheme (ESP32 supports line-fitting only)
    adc_cali_line_fitting_config_t cali_cfg = {
        .unit_id  = BATT_ADC_UNIT,
        .atten    = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    if (adc_cali_create_scheme_line_fitting(&cali_cfg, &cali_handle) == ESP_OK) {
        cali_enabled = true;
        ESP_LOGI(TAG, "ADC calibration enabled (line fitting)");
    } else {
        ESP_LOGW(TAG, "ADC calibration unavailable — using raw conversion");
    }

    return ESP_OK;
}

float battery_read_voltage(void)
{
    int raw = 0;
    if (adc_oneshot_read(adc_handle, BATT_ADC_CHANNEL, &raw) != ESP_OK) {
        return 0.0f;
    }

    int millivolts;
    if (cali_enabled &&
        adc_cali_raw_to_voltage(cali_handle, raw, &millivolts) == ESP_OK) {
        // Calibrated millivolts at the ADC pin → multiply by divider ratio
        return (millivolts * BATT_DIVIDER_RATIO) / 1000.0f;
    } else {
        // Fallback: 12-bit raw, 3.3V reference
        return (raw / 4095.0f) * 3.3f * BATT_DIVIDER_RATIO;
    }
}
