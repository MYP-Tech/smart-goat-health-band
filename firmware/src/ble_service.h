/*
 * BLE GATT server — exposes one characteristic that streams telemetry JSON.
 * Built on Bluedroid (ESP-IDF native BLE host stack).
 */
#pragma once
#include <stddef.h>
#include <stdint.h>
#include "esp_err.h"

esp_err_t ble_service_init(void);
void      ble_service_notify(const uint8_t *data, size_t len);
