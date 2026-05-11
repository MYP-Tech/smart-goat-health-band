/*
 * GoatBand MVP-1 — pin map and constants
 * All hardware pin assignments live here. Change one place, propagates everywhere.
 */
#pragma once

// --------------- I2C bus (MPU-6050) ---------------
#define I2C_MASTER_SDA_IO       21
#define I2C_MASTER_SCL_IO       22
#define I2C_MASTER_NUM          0
#define I2C_MASTER_FREQ_HZ      400000
#define MPU6050_I2C_ADDR        0x68

// --------------- 1-Wire bus (DS18B20) ---------------
#define DS18B20_GPIO            4

// --------------- ADC (battery monitoring) ---------------
#define BATT_ADC_CHANNEL        ADC_CHANNEL_7   // GPIO 35 = ADC1_CH7
#define BATT_ADC_UNIT           ADC_UNIT_1
#define BATT_DIVIDER_RATIO      2.0f            // 100k / 100k divider, doubles measured

// --------------- LoRa SX1276 (MVP-4 onward) ---------------
#define LORA_SPI_HOST           SPI2_HOST
#define LORA_PIN_MOSI           23
#define LORA_PIN_MISO           19
#define LORA_PIN_SCK            18
#define LORA_PIN_CS             5
#define LORA_PIN_RST            14
#define LORA_PIN_DIO0           26

// --------------- Sampling timing ---------------
#define MOTION_SAMPLE_HZ        20      // 20 Hz motion sampling
#define WINDOW_DURATION_MS      30000   // emit a packet every 30 seconds
#define TEMP_SAMPLE_INTERVAL_MS 30000

// --------------- BLE ---------------
#define BLE_DEVICE_NAME         "GoatBand-001"
#define BLE_SERVICE_UUID        0x180D  // example, replace with custom 128-bit later
#define BLE_CHAR_UUID           0x2A37

// --------------- NVS namespace ---------------
#define NVS_NAMESPACE           "goatband"
