/* MPU-6050 driver — ESP-IDF native I2C, no Arduino abstraction */
#pragma once
#include "esp_err.h"

typedef struct {
    float ax, ay, az;   // accel in m/s²
    float gx, gy, gz;   // gyro in deg/s
    float temp_c;
} mpu6050_data_t;

esp_err_t mpu6050_init(void);
esp_err_t mpu6050_read(mpu6050_data_t *out);
