# GoatBand: 5-MVP Development Plan

> **Hardware · Firmware · Pilot test on real goats**

## Configuration Locked for This Plan

| Decision | Choice |
| :--- | :--- |
| **Radio** | BLE + LoRa (BLE for nearby phone, LoRa for shed range) |
| **Power** | Battery-only with TP4056 charging in-place (no swap dock for MVP) |
| **Sensors** | MPU-6050 motion + DS18B20 temperature from MVP-1 |
| **Test scale** | 5 bands on real goats |
| **Total budget** | ~₹15,000 for 5 bands + tools + Pi hub |

---

## 1. Executive Summary

This document is the build plan for the first 5 GoatBand MVPs. It is derived from the project PDF and the reference visuals, and locked against three decisions made up front: BLE+LoRa as the radio, in-place TP4056 charging for the MVP (battery swap deferred), and motion + temperature sensors from MVP-1.

The PDF describes a Phase 3 system. This plan derisks it in five stages, each producing data that feeds the next. By the end of MVP-5, you will have 5 bands on real goats, baseline learning validated against actual behavior, and a phone app that fires real alerts.

### Why 5 MVPs and not one big build?

*   **Goats chew, scratch, sleep, and rub against fences.** None of this is in your spec. MVP-2 exists to discover those edge cases on one tame goat before you scale.
*   **LoRa range claims of 2 km are optimistic** in a dense farm with metal sheds. MVP-4 measures the real range before you commit to 1 hub per farm.
*   **The activity scoring algorithm is your differentiator.** MVP-3 validates that per-goat baselines actually work — if the algorithm is wrong, everything downstream is wrong.
*   **Battery life claims need real data.** Every MVP measures actual current draw under realistic conditions.

---

## 2. Architecture Diagrams

Two complementary views of the system. The component view answers "what are the moving parts and how do they communicate?" The system view answers "where does each part run and how does it scale?"

### 2.1 Component Architecture

Six major components — neckband, shed hub, cloud backend, mobile app, charging dock, and farmer — with named protocols on every connector. Solid arrows are production paths, dashed is the BLE-only path used during MVP-1 to MVP-3, dotted is physical/out-of-band relationship (battery swap).

![Component Architecture](diagrams/01_component_architecture.png)

### 2.2 System Architecture (Deployment View)

Four numbered layers stacked from physical edge devices at the top down to the farmer's phone at the bottom. Each layer is a separate deployment unit: Layer 1 ships on hardware, Layer 2 ships on the Pi image, Layers 3 and 4 ship via CI/CD.

![System Architecture](diagrams/02_system_architecture.png)

---

## 3. The 5 MVPs

### MVP-1 — Bench Prototype, Single Band
**Goal**: Prove every component works on a breadboard, end-to-end, before any soldering.

| Item | Detail |
| :--- | :--- |
| **Duration** | Week 1–2 |
| **Units** | 1 breadboard build |
| **Hardware** | ESP32 + MPU-6050 + DS18B20 + TP4056 + 18650 + voltage divider |
| **Firmware** | Read sensors every 30s · log to serial · BLE characteristic exposes JSON |
| **Test setup** | Shake the band by hand · breathe on the temp sensor · watch numbers change |
| **Pass criteria** | Phone connects via BLE · receives a packet every 30s · numbers move when you move it |
| **Cost** | ₹2,200 for one unit |

### MVP-2 — One Goat, 72 Hours
**Goal**: Discover what real goat behavior looks like in raw data, before you write any scoring logic.

| Item | Detail |
| :--- | :--- |
| **Duration** | Week 3 |
| **Units** | 1 PCB-soldered band in basic enclosure |
| **Hardware** | Same as MVP-1 but soldered onto a perfboard, sealed in IP54 box |
| **Firmware** | Logs raw motion + temp every 10s to onboard flash · syncs to phone over BLE when in range |
| **Test setup** | One tame, healthy goat · 72 hours continuous · phone visits twice a day to download data |
| **Pass criteria** | Continuous logs for 72h with no dropouts · battery survives the duration |
| **What you'll find**| Sleeping ≠ sick · chewing creates rhythmic motion · sun-bathing spikes temp · mounting/scratching create false-positive movement spikes |

### MVP-3 — 5 Goats, Baseline Learning
**Goal**: Validate that per-goat baseline learning works in the real world.

| Item | Detail |
| :--- | :--- |
| **Duration** | Week 4–5 |
| **Units** | 5 PCB bands in proper enclosures |
| **Hardware** | 5× MVP-2 units with cleaned-up form factor |
| **Firmware** | First 5 days = baseline learning · day 6 onward = activity score 0–100 · still BLE only |
| **Test setup** | 5 goats of mixed temperaments (active, lazy, mid) · phone collects data daily |
| **Pass criteria** | Each goat's baseline is measurably different · no false alerts on lazy goats · score correlates with vet's visual assessment |
| **What this proves**| Your core algorithm works. If MVP-3 fails, MVP-4 and MVP-5 are pointless. |

### MVP-4 — LoRa Hub + Cloud Sync
**Goal**: Validate the LoRa range claims and the offline-buffering behavior.

| Item | Detail |
| :--- | :--- |
| **Duration** | Week 6–8 |
| **Units** | 5 bands + 1 Raspberry Pi shed hub |
| **Hardware** | MVP-3 bands + SX1276 LoRa daughterboard · Pi Zero 2W with LoRa hat · WiFi to cloud |
| **Firmware** | Bands send LoRa every 5 min · Pi forwards to cloud · cloud stores in time-series DB |
| **Test setup** | Walk one band away from the hub in 100m increments · note packet loss · test with metal shed in line of sight |
| **Pass criteria** | 200m reliable through one shed wall · 500m line-of-sight · no data lost when WiFi drops for 1 hour |
| **What you'll find**| Real-world LoRa range is usually 30–60% of spec sheet |

### MVP-5 — Mobile App + Farmer Field Test
**Goal**: Full loop with a real farmer making decisions from your alerts.

| Item | Detail |
| :--- | :--- |
| **Duration** | Week 9–12 |
| **Units** | 5 bands + hub + mobile app (Flutter or React Native) |
| **App screens** | Dashboard · alerts · individual goat profile · weight log · vaccination log |
| **Test setup** | 5 bands on 5 goats including 1 you intentionally don't feed for a day to simulate poor health |
| **Pass criteria** | Farmer notices the under-fed goat in app before noticing visually · 0 false alerts in 7 days · 100% uptime |
| **What this proves**| The product actually changes farmer behavior. This is what investors and customers care about. |

---

## 4. Hardware — Bill of Materials

### Per-Band Components (MVP-1 Spec)

| Component | Specific part | Qty | Cost (₹) | Source |
| :--- | :--- | :--- | :--- | :--- |
| **Microcontroller** | ESP32 NodeMCU-32S (38-pin) | 1 | 350 | Robu, Amazon |
| **Motion sensor** | MPU-6050 GY-521 module | 1 | 80 | Robu |
| **Temperature sensor** | DS18B20 waterproof probe | 1 | 120 | Robu |
| **Charger module** | TP4056 with USB-C + protection | 1 | 50 | Robu |
| **Battery** | 18650 Li-ion 2600 mAh (Samsung 26F) | 1 | 300 | LiPol |
| **Battery holder** | 18650 single-cell holder, soldered | 1 | 30 | Robu |
| **Resistor** | 4.7kΩ 1/4W (DS18B20 pullup) | 1 | 1 | Local |
| **Resistor pair** | 100kΩ 1/4W (battery divider) | 2 | 2 | Local |
| **Slide switch** | SPDT mini slide switch | 1 | 10 | Local |
| **Perfboard** | 5×7cm prototype PCB | 1 | 30 | Local |
| **Wire** | 30 AWG silicone hookup wire | 1m | 20 | Local |
| **Enclosure** | IP54 ABS box ~70×50×25mm | 1 | 120 | Robu |
| **Strap** | 25mm nylon webbing + buckle | 1 | 60 | Local |
| **Silica gel** | Moisture pack 2g | 1 | 5 | Local |
| **LoRa module (MVP-4+)** | SX1276 RA-02 433MHz | 1 | 350 | Robu |
| **Per-band total (no LoRa)** | | | **1,178** | |
| **Per-band total (with LoRa)** | | | **1,528** | |

### Shed Hub (MVP-4 Onward)

| Component | Cost (₹) |
| :--- | :--- |
| Raspberry Pi Zero 2W | 1,800 |
| microSD 32GB | 300 |
| SX1276 LoRa hat | 600 |
| 5V 3A USB-C adapter + case | 400 |
| **Hub total** | **3,100** |

### Total Project Cost — 5 MVP Bands + Hub

| Item | Qty | Unit (₹) | Total (₹) |
| :--- | :--- | :--- | :--- |
| MVP-1/2/3 bands (no LoRa) | 5 | 1,178 | 5,890 |
| LoRa modules added in MVP-4 | 5 | 350 | 1,750 |
| Shed hub | 1 | 3,100 | 3,100 |
| Tools (soldering iron, multimeter, breadboard) | 1 | 2,500 | 2,500 |
| Spare components 20% buffer | — | — | 1,800 |
| **Grand total** | | | **15,040** |

---

## 5. Wiring Map — Pin by Pin

The Tinkercad-style breadboard layout below shows the physical placement of every component. Use this as your visual reference; the tables that follow give exact pin-to-pin mappings.

![Breadboard Layout](diagrams/03_breadboard_tinkercad.png)

This matches the circuit diagram. Every connection in the MVP-1 build:

### Power Rail

| From | To | Wire |
| :--- | :--- | :--- |
| 18650 (+) | TP4056 B+ | Red, 22 AWG |
| 18650 (−) | TP4056 B− | Black, 22 AWG |
| TP4056 OUT+ | SPDT switch (common) | Red, 22 AWG |
| SPDT switch (out) | ESP32 VIN | Red, 22 AWG |
| TP4056 OUT− | ESP32 GND | Black, 22 AWG |

### Sensors

| From | To | Notes |
| :--- | :--- | :--- |
| ESP32 3V3 | MPU-6050 VCC, DS18B20 VCC | Common 3.3V rail |
| ESP32 GND | MPU-6050 GND, DS18B20 GND | Common ground |
| ESP32 GPIO 21 | MPU-6050 SDA | I²C data |
| ESP32 GPIO 22 | MPU-6050 SCL | I²C clock |
| ESP32 GPIO 4 | DS18B20 DATA | 1-Wire — needs 4.7kΩ pullup to 3V3 |

### Battery Monitor

| From | To | Notes |
| :--- | :--- | :--- |
| Battery + (after switch) | 100kΩ → midpoint → 100kΩ → GND | Half voltage divider |
| Divider midpoint | ESP32 GPIO 35 | ADC1_CH7 — read raw, multiply by 2 |

### LoRa (MVP-4 Onward)

| From | To |
| :--- | :--- |
| ESP32 GPIO 5 | RA-02 NSS (CS) |
| ESP32 GPIO 18 | RA-02 SCK |
| ESP32 GPIO 19 | RA-02 MISO |
| ESP32 GPIO 23 | RA-02 MOSI |
| ESP32 GPIO 14 | RA-02 RST |
| ESP32 GPIO 26 | RA-02 DIO0 |
| ESP32 3V3 | RA-02 VCC |
| ESP32 GND | RA-02 GND |

---

## 6. MVP-1 Firmware — ESP-IDF (No Arduino)

This project uses pure ESP-IDF on the ESP32 NodeMCU-32S. No Arduino IDE, no Arduino framework, no Wire/OneWire/Adafruit libraries. Build with PlatformIO inside VS Code.

### Why ESP-IDF and Not Arduino
*   Direct access to FreeRTOS, NVS, ADC v5 driver, and Bluedroid — the same stack Espressif uses internally
*   Finer power control — critical when chasing the 50-day battery target
*   Smaller binary and explicit task model — easier to reason about than `setup()`/`loop()`
*   Same tooling carries through MVP-5 — no rewrite later when you need OTA, deep sleep, or custom partitions

### Project Layout
```
firmware/
├── platformio.ini       board: nodemcu-32s, framework: espidf
├── partitions.csv       custom flash layout (larger NVS for baselines)
├── CMakeLists.txt       ESP-IDF requirement
├── include/
│   └── pinmap.h         single source of truth for hardware pins
└── src/
    ├── CMakeLists.txt
    ├── main.c           app_main + 2 FreeRTOS tasks
    ├── mpu6050.c/.h     I²C motion sensor driver
    ├── ds18b20.c/.h     1-Wire temperature driver
    ├── battery.c/.h     ADC v5 oneshot battery monitor
    ├── ble_service.c/.h Bluedroid GATT server
    └── activity_score.c/.h  MVP-1 placeholder, swapped in MVP-3
```

### Setup Steps
1. Install VS Code
2. Install the PlatformIO IDE extension
3. Open the `firmware/` folder as a PlatformIO project
4. PlatformIO downloads ESP-IDF v5.x automatically on first build — no manual toolchain install
5. Connect the ESP32 over USB-C, then Build → Upload → Monitor from the PlatformIO sidebar

### Architecture — What Runs Where

| Task | Core | Priority | Job |
| :--- | :--- | :--- | :--- |
| `motion_task` | 1 | 6 (high) | Samples MPU-6050 at 20 Hz, accumulates magnitude minus gravity |
| `telemetry_task` | 0 | 4 (normal) | Every 30s reads temp + battery, computes score, sends BLE notify |
| BLE host stack | 0 | Bluedroid | Handles GAP advertising and GATT client connections |

### Code Snippets

**main.c (Entry Point)**
```c
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/i2c.h"
#include "pinmap.h"
#include "mpu6050.h"
#include "ds18b20.h"
#include "battery.h"
#include "ble_service.h"
#include "activity_score.h"
 
static SemaphoreHandle_t state_mutex;
static float    motion_accum = 0.0f;
static uint32_t motion_count = 0;
 
static void motion_task(void *arg) {
    const TickType_t period = pdMS_TO_TICKS(1000 / MOTION_SAMPLE_HZ);
    mpu6050_data_t d;
    while (1) {
        if (mpu6050_read(&d) == ESP_OK) {
            float mag    = sqrtf(d.ax*d.ax + d.ay*d.ay + d.az*d.az);
            float motion = fabsf(mag - 9.81f);     // strip gravity
            xSemaphoreTake(state_mutex, portMAX_DELAY);
            motion_accum += motion;
            motion_count++;
            xSemaphoreGive(state_mutex);
        }
        vTaskDelay(period);
    }
}
 
static void telemetry_task(void *arg) {
    char json[256];
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(WINDOW_DURATION_MS));
 
        float avg = 0.0f;
        xSemaphoreTake(state_mutex, portMAX_DELAY);
        if (motion_count > 0) avg = motion_accum / motion_count;
        motion_accum = 0.0f; motion_count = 0;
        xSemaphoreGive(state_mutex);
 
        float t  = ds18b20_read_temp();
        float v  = battery_read_voltage();
        int   s  = activity_score_compute(avg);
 
        int n = snprintf(json, sizeof(json),
            "{\"t\":%lu,\"motion\":%.3f,\"temp\":%.2f,"
            "\"batt\":%.2f,\"score\":%d}",
            (unsigned long)(esp_log_timestamp()/1000), avg, t, v, s);
 
        ESP_LOGI("goatband", "%s", json);
        if (n > 0) ble_service_notify((uint8_t*)json, n);
    }
}
 
void app_main(void) {
    nvs_flash_init();
    i2c_master_init();          // brings up I²C bus on GPIO 21/22
    mpu6050_init();
    ds18b20_init();
    battery_init();
    ble_service_init();
 
    state_mutex = xSemaphoreCreateMutex();
 
    xTaskCreatePinnedToCore(motion_task,    "motion",    4096, NULL, 6, NULL, 1);
    xTaskCreatePinnedToCore(telemetry_task, "telemetry", 4096, NULL, 4, NULL, 0);
}
```

**MPU-6050 Driver — Direct Register Access**
No Adafruit library. The driver writes config registers directly and reads 14 bytes (accel + temp + gyro) in a single I²C burst:
```c
// MPU register addresses
#define MPU_REG_PWR_MGMT_1   0x6B
#define MPU_REG_ACCEL_CONFIG 0x1C
#define MPU_REG_GYRO_CONFIG  0x1B
#define MPU_REG_ACCEL_XOUT_H 0x3B
 
esp_err_t mpu6050_init(void) {
    mpu_write_reg(MPU_REG_PWR_MGMT_1, 0x00);    // wake from sleep
    mpu_write_reg(MPU_REG_ACCEL_CONFIG, 0x08);  // ±4g
    mpu_write_reg(MPU_REG_GYRO_CONFIG,  0x08);  // ±500°/s
    return ESP_OK;
}
 
esp_err_t mpu6050_read(mpu6050_data_t *out) {
    uint8_t raw[14];
    mpu_read_regs(MPU_REG_ACCEL_XOUT_H, raw, sizeof(raw));
 
    int16_t ax = (raw[0]<<8)|raw[1];
    int16_t ay = (raw[2]<<8)|raw[3];
    int16_t az = (raw[4]<<8)|raw[5];
 
    out->ax = ax * (9.81f / 8192.0f);   // ±4g → m/s²
    out->ay = ay * (9.81f / 8192.0f);
    out->az = az * (9.81f / 8192.0f);
    return ESP_OK;
}
```

**DS18B20 Driver — Bit-Banged 1-Wire**
ESP-IDF doesn't ship a 1-Wire driver in core, so this is a minimal bit-banged implementation using `ets_delay_us` for timing. Pull-up resistor (4.7kΩ from DATA to 3V3) is mandatory.
```c
float ds18b20_read_temp(void) {
    if (!ow_reset()) return -127.0f;
    ow_write_byte(0xCC);   // SKIP ROM
    ow_write_byte(0x44);   // CONVERT T
 
    vTaskDelay(pdMS_TO_TICKS(800));   // 12-bit conversion needs 750ms
 
    if (!ow_reset()) return -127.0f;
    ow_write_byte(0xCC);
    ow_write_byte(0xBE);   // READ SCRATCHPAD
 
    uint8_t lsb = ow_read_byte();
    uint8_t msb = ow_read_byte();
    int16_t raw = (msb << 8) | lsb;
    return raw * 0.0625f;   // 1 LSB = 0.0625 °C
}
```

**Battery Monitor — ADC v5 Oneshot**
The legacy `adc1_get_raw()` API is deprecated in IDF v5. This module uses the new oneshot driver with line-fitting calibration where supported.
```c
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
 
float battery_read_voltage(void) {
    int raw;
    adc_oneshot_read(adc_handle, BATT_ADC_CHANNEL, &raw);
 
    int mv;
    if (cali_enabled &&
        adc_cali_raw_to_voltage(cali_handle, raw, &mv) == ESP_OK) {
        return (mv * BATT_DIVIDER_RATIO) / 1000.0f;   // calibrated path
    }
    return (raw / 4095.0f) * 3.3f * BATT_DIVIDER_RATIO; // fallback
}
```

**BLE GATT Server — Bluedroid**
One custom 128-bit service with one characteristic supporting READ + NOTIFY. Phone connects via nRF Connect (Nordic) and subscribes to the characteristic to see telemetry stream:
```c
// 128-bit UUIDs — service and characteristic
static const uint8_t SVC_UUID[16]  = { 0xF0, 0xCD, 0xAB, 0x89, ... };
static const uint8_t CHAR_UUID[16] = { 0xF1, 0xCD, 0xAB, 0x89, ... };
 
esp_err_t ble_service_init(void) {
    esp_bt_controller_init(&bt_cfg);
    esp_bt_controller_enable(ESP_BT_MODE_BLE);
    esp_bluedroid_init();
    esp_bluedroid_enable();
 
    esp_ble_gap_register_callback(gap_cb);
    esp_ble_gatts_register_callback(gatts_cb);
    esp_ble_gatts_app_register(APP_ID);
    return ESP_OK;
}
 
void ble_service_notify(const uint8_t *data, size_t len) {
    if (g_conn_id == 0xFFFF) return;
    esp_ble_gatts_send_indicate(g_gatts_if, g_conn_id,
                                 g_char_handle, len,
                                 (uint8_t*)data, false);
}
```

### How to Test (No Goat Needed)
*   Flash and open the PlatformIO Monitor at 115200 baud — JSON should appear every 30 seconds
*   Install nRF Connect on your phone, scan for `GoatBand-001`, connect
*   Subscribe to the custom characteristic — telemetry notifications stream in real-time
*   Shake the breadboard — the `motion` and `score` fields should jump immediately
*   Pinch the DS18B20 probe — `temp` should rise toward body temperature within ~1 second
*   Multimeter check: `batt` field should match the actual 18650 voltage to within 0.05V

### What MVP-1 Firmware Does NOT Do
These are intentionally deferred. Adding them too early creates noise that hides the real bugs:
*   **Per-goat baseline learning** — added in MVP-3 by extending `activity_score.c`
*   **NVS persistence of baselines** — added in MVP-3
*   **LoRa transmission** — added in MVP-4 (pins are already mapped in `pinmap.h`)
*   **Deep sleep between samples** — added in MVP-2 once power profile is measured
*   **OTA updates** — added in MVP-5

---

## 7. MVP-3 Baseline-Learning Algorithm

This is the heart of the product. The algorithm runs on each band, not on the cloud.

### Algorithm in Plain English
*   **Days 1–5**: record every 30-min activity bucket. At end of day 5, compute mean and stdev for each hour-of-day (24 hourly profiles).
*   **Day 6+**: every 30 min, compute current activity. Compare to that hour's baseline mean.
*   If current is below baseline mean by more than 1.5× stdev → that's a **'low'** reading.
*   **3 consecutive low readings** (~90 min) → fire **WARNING** alert.
*   **5 consecutive low readings + temp >40°C** → fire **CRITICAL** alert.
*   Reset the counter on any normal-or-above reading. This avoids false alarms from afternoon naps.

### Pseudocode
```c
// per-goat persistent state (stored in ESP32 NVS)
struct Baseline {
  float hourlyMean[24];
  float hourlyStdev[24];
  bool learned;          // true after day 5
  uint8_t consecLow;     // consecutive low readings
};
 
void evaluateActivity(int currentHour, float currentActivity, float currentTemp) {
  if (!baseline.learned) {
    accumulateLearningSample(currentHour, currentActivity);
    return;
  }
 
  float threshold = baseline.hourlyMean[currentHour] -
                    1.5 * baseline.hourlyStdev[currentHour];
 
  if (currentActivity < threshold) {
    baseline.consecLow++;
  } else {
    baseline.consecLow = 0;
  }
 
  AlertLevel level = NORMAL;
  if (baseline.consecLow >= 3) level = WARNING;
  if (baseline.consecLow >= 5 && currentTemp > 40.0) level = CRITICAL;
 
  publishAlert(level);
}
```

---

## 8. Build Sequence — Week by Week

| Week | Deliverable | Risk to Watch |
| :--- | :--- | :--- |
| **1** | Order all components, set up Arduino IDE, breadboard MVP-1 | Wrong ESP32 variant — buy NodeMCU-32S not generic |
| **2** | MVP-1 firmware running, BLE working, JSON streaming | Library version mismatches |
| **3** | MVP-2 — solder onto perfboard, enclosure, 1 goat 72h | Strap rubs raw — sew a fabric liner |
| **4** | MVP-3 build × 5 units | Soldering quality on small batches |
| **5** | MVP-3 deploy + baseline learning runs | Baselines noisy — extend learning to 7 days if needed |
| **6** | MVP-4 hub build, Pi LoRa receiving | Antenna placement matters more than range |
| **7** | MVP-4 cloud sync + range test | WiFi at the farm. 4G dongle as fallback. |
| **8** | MVP-4 review + fix issues | Buffer week — every project needs one |
| **9** | MVP-5 mobile app skeleton | Pick Flutter — single codebase for Android+iOS |
| **10** | MVP-5 dashboard + alerts wired | Don't over-design — match app reference image |
| **11** | MVP-5 farmer field test | Farmer has to actually use it. Sit with them. |
| **12** | MVP-5 review, write up findings | What broke is more valuable than what worked |

---

## 9. Risks and What to Do About Them

| Risk | How to detect it early | What to do |
| :--- | :--- | :--- |
| Strap chafes goat's neck | Visible irritation by day 2 of MVP-2 | Sew soft cotton liner inside strap |
| Battery dies faster than spec | MVP-1 measures actual mAh draw | Increase sleep duration between samples |
| LoRa range less than 200m | MVP-4 walk test | Add second hub or repeater |
| Baseline learning is noisy | MVP-3 produces too many warnings | Extend learning to 7 days, increase stdev multiplier |
| Farmer ignores app alerts | MVP-5 — alerts don't lead to vet visits | Switch to SMS instead of push, or call automatically |
| Goats remove bands | Find unworn band on the ground | Tighter buckle, anti-tamper design |

---

## 10. Quick-Reference Checklists

### Before Flashing Any Band — Checklist
*   [ ] Battery voltage between 3.4V and 4.2V (multimeter check)
*   [ ] All ground rails connected to single point
*   [ ] 4.7kΩ pullup on DS18B20 data line — easy to forget
*   [ ] ESP32 board variant set to NodeMCU-32S in Arduino IDE
*   [ ] Upload speed 460800 (or 921600) — slower if errors

### Before Deploying on a Goat — Checklist
*   [ ] Enclosure sealed, silica gel inside
*   [ ] Strap fits 2 fingers under — not tight, not loose
*   [ ] Smooth all edges that contact skin
*   [ ] Logged 1 hour on bench without errors
*   [ ] Battery >80% before deployment
*   [ ] Vet has approved the test

---

## 11. What to Build After MVP-5

If MVP-5 succeeds (farmer notices the under-fed goat from your alert before noticing visually), you have product-market fit signal. Next steps:

*   **Custom PCB design** — perfboard works for 5, not for 50. Use KiCad, fab at JLCPCB.
*   **Battery-swap mechanism** (the original PDF spec) — only when reuse becomes necessary
*   **Injection-molded enclosure** — only at 100+ units
*   **Cellular fallback in the band itself** for farms without sheds — defer until customer asks
*   **Health AI** — feed the time-series data into a real ML model. You'll have months of data by then.

***
*End of plan.*
