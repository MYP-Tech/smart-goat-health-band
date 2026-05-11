idf_component_register(
    SRCS "main.c"
         "mpu6050.c"
         "ds18b20.c"
         "ble_service.c"
         "battery.c"
         "activity_score.c"
    INCLUDE_DIRS "."
                 "../include"
    REQUIRES driver
             nvs_flash
             esp_adc
             bt
             json
)
