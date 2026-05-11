# GoatBand MVP-1 firmware

Pure ESP-IDF on ESP32 NodeMCU-32S. **No Arduino**, no Arduino IDE.
Built with PlatformIO inside VS Code.

## Project layout

```
firmware/
├── platformio.ini      ← board, framework, build settings
├── partitions.csv      ← custom flash layout (extra NVS for baselines)
├── CMakeLists.txt      ← required by ESP-IDF build system
├── include/
│   └── pinmap.h        ← single source of truth for hardware pins
└── src/
    ├── CMakeLists.txt
    ├── main.c          ← app_main + FreeRTOS tasks
    ├── mpu6050.c/.h    ← I²C motion sensor driver
    ├── ds18b20.c/.h    ← 1-Wire temperature driver
    ├── battery.c/.h    ← ADC battery voltage monitor
    ├── ble_service.c/.h ← Bluedroid GATT server
    └── activity_score.c/.h ← MVP-1 placeholder (replaced in MVP-3)
```

## How to build and flash

1. Install **VS Code** + the **PlatformIO IDE** extension
2. Open the `firmware/` folder as a PlatformIO project
3. PlatformIO will download ESP-IDF v5.x automatically on first build
4. Connect the ESP32 over USB-C
5. From the PlatformIO sidebar, run:
   - **Build** — compile firmware
   - **Upload** — flash to the device
   - **Monitor** — open serial output at 115200 baud

## How to test (no goat needed)

1. After flashing, open the Monitor — you should see JSON every 30 seconds:

   ```json
   {"t":30,"motion":0.018,"temp":27.43,"batt":3.92,"score":0}
   ```

2. Install **nRF Connect** (Nordic Semiconductor) on your phone
3. Scan for `GoatBand-001` and connect
4. Subscribe to the custom characteristic (UUID ending `...DEF1`)
5. Shake the breadboard — `motion` and `score` should jump in real time
6. Pinch the temperature probe — `temp` should rise toward body temperature

## What MVP-1 firmware does NOT do

These are intentionally deferred. See the 5-MVP plan document for context.

- **Per-goat baseline learning** — added in MVP-3 (extends `activity_score.c`)
- **NVS persistence of baselines** — added in MVP-3
- **LoRa transmission to shed hub** — added in MVP-4 (uses pins already mapped in `pinmap.h`)
- **Deep sleep between samples** — added in MVP-2 once power profile is measured
- **OTA updates** — added in MVP-5

## Why ESP-IDF and not Arduino

- Direct access to FreeRTOS, NVS, ADC v5 driver, Bluedroid stack
- Smaller binary, finer power control (matters when you're chasing 50-day battery)
- No hidden setup/loop abstraction — explicit task model, easier to reason about
- Same toolchain Espressif uses internally → fewer surprises in production
