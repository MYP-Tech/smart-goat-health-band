/* Battery voltage monitor — reads divider on GPIO 35 via ADC1 oneshot */
#pragma once
#include "esp_err.h"

esp_err_t battery_init(void);
float     battery_read_voltage(void);   // returns volts
