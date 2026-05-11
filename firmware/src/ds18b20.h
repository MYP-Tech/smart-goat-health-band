/* DS18B20 — 1-Wire temperature sensor, ESP-IDF native */
#pragma once
#include "esp_err.h"

esp_err_t ds18b20_init(void);
float     ds18b20_read_temp(void);   // returns °C, or -127.0 on error
