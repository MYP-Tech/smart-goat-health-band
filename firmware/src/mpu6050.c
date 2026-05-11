/* MPU-6050 — direct register-level driver over ESP-IDF I2C */
#include "mpu6050.h"
#include "pinmap.h"
#include "driver/i2c.h"
#include "esp_log.h"

#define MPU_REG_PWR_MGMT_1   0x6B
#define MPU_REG_ACCEL_CONFIG 0x1C
#define MPU_REG_GYRO_CONFIG  0x1B
#define MPU_REG_ACCEL_XOUT_H 0x3B
#define MPU_REG_WHO_AM_I     0x75

static const char *TAG = "mpu6050";

// Scale factor for ±4g range: 8192 LSB/g; convert to m/s² with 9.81
#define ACCEL_SCALE_LSB_TO_MS2  (9.81f / 8192.0f)
// Scale factor for ±500°/s range: 65.5 LSB/(°/s)
#define GYRO_SCALE_LSB_TO_DPS   (1.0f / 65.5f)

static esp_err_t mpu_write_reg(uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = { reg, val };
    return i2c_master_write_to_device(I2C_MASTER_NUM, MPU6050_I2C_ADDR,
                                       buf, 2, pdMS_TO_TICKS(100));
}

static esp_err_t mpu_read_regs(uint8_t reg, uint8_t *buf, size_t len)
{
    return i2c_master_write_read_device(I2C_MASTER_NUM, MPU6050_I2C_ADDR,
                                         &reg, 1, buf, len, pdMS_TO_TICKS(100));
}

esp_err_t mpu6050_init(void)
{
    uint8_t whoami = 0;
    esp_err_t err = mpu_read_regs(MPU_REG_WHO_AM_I, &whoami, 1);
    if (err != ESP_OK || whoami != 0x68) {
        ESP_LOGE(TAG, "WHO_AM_I check failed: 0x%02X (err %d)", whoami, err);
        return ESP_FAIL;
    }

    // Wake from sleep, select internal 8 MHz oscillator
    ESP_ERROR_CHECK(mpu_write_reg(MPU_REG_PWR_MGMT_1, 0x00));

    // ±4g range
    ESP_ERROR_CHECK(mpu_write_reg(MPU_REG_ACCEL_CONFIG, 0x08));

    // ±500°/s range
    ESP_ERROR_CHECK(mpu_write_reg(MPU_REG_GYRO_CONFIG, 0x08));

    ESP_LOGI(TAG, "MPU-6050 initialized");
    return ESP_OK;
}

esp_err_t mpu6050_read(mpu6050_data_t *out)
{
    uint8_t raw[14];
    esp_err_t err = mpu_read_regs(MPU_REG_ACCEL_XOUT_H, raw, sizeof(raw));
    if (err != ESP_OK) return err;

    int16_t ax = (raw[0]  << 8) | raw[1];
    int16_t ay = (raw[2]  << 8) | raw[3];
    int16_t az = (raw[4]  << 8) | raw[5];
    int16_t tr = (raw[6]  << 8) | raw[7];
    int16_t gx = (raw[8]  << 8) | raw[9];
    int16_t gy = (raw[10] << 8) | raw[11];
    int16_t gz = (raw[12] << 8) | raw[13];

    out->ax = ax * ACCEL_SCALE_LSB_TO_MS2;
    out->ay = ay * ACCEL_SCALE_LSB_TO_MS2;
    out->az = az * ACCEL_SCALE_LSB_TO_MS2;
    out->gx = gx * GYRO_SCALE_LSB_TO_DPS;
    out->gy = gy * GYRO_SCALE_LSB_TO_DPS;
    out->gz = gz * GYRO_SCALE_LSB_TO_DPS;
    out->temp_c = (tr / 340.0f) + 36.53f;

    return ESP_OK;
}
