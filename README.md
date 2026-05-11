# GoatBand

> Smart neckband for early illness detection in commercial goat farms.
>
> Hardware · Firmware · Pilot test on real goats · 5-MVP development plan

## What Is GoatBand?

GoatBand is a wearable neckband that continuously monitors goat activity and body temperature, learns each animal's individual behavioral baseline, and alerts farmers when a goat's behavior deviates from normal — before visible symptoms appear.

The project follows a **5-MVP incremental development approach**: each stage produces data that feeds the next, de-risking hardware, firmware, algorithm, radio, and user experience in sequence.

## Architecture

### Component Architecture

Six major components — neckband, shed hub, cloud backend, mobile app, charging dock, and farmer — with named protocols on every connector.

![Component Architecture — what talks to what](diagrams/01_component_architecture.png)

### System Architecture

Four numbered layers from edge devices down to the farmer's phone. Each layer is a separate deployment unit.

![System Architecture — layered deployment view](diagrams/02_system_architecture.png)

### Breadboard Layout (MVP-1)

![Breadboard Layout — physical wiring reference](diagrams/03_breadboard_tinkercad.png)

## The 5 MVPs

| MVP | Focus | Duration | Status |
|-----|-------|----------|--------|
| **MVP-1** | Bench prototype — sensors + BLE on breadboard | Week 1–2 | ✅ Current |
| **MVP-2** | One goat, 72 hours — discover real behavior | Week 3 | 🔜 Next |
| **MVP-3** | 5 goats, per-goat baseline learning | Week 4–5 | 📋 Planned |
| **MVP-4** | LoRa hub + cloud sync + range testing | Week 6–8 | 📋 Planned |
| **MVP-5** | Flutter app + farmer field test | Week 9–12 | 📋 Planned |

## Repository Structure

```
GoatBand_release/
├── README.md                          ← You are here
├── GoatBand_5MVP_Plan.docx            ← Master plan document with embedded diagrams
├── docs/                              ← Technical documentation
│   ├── 01_PROJECT_OVERVIEW.md         ← Problem, BOM, MVP roadmap, build sequence
│   ├── 02_SYSTEM_ARCHITECTURE.md      ← Layers, wiring map, component diagrams
│   ├── 03_FIRMWARE_DESIGN.md          ← FreeRTOS tasks, boot sequence, concurrency
│   ├── 04_MODULE_REFERENCE.md         ← Per-module API docs with sequence diagrams
│   ├── 05_DATA_FLOW_AND_PROTOCOLS.md  ← I2C, 1-Wire, BLE, data pipeline, alert logic
│   └── 06_BUILD_AND_DEPLOYMENT.md     ← Build, flash, test, troubleshoot, checklists
├── diagrams/                          ← Architecture diagrams (SVG + PNG)
│   ├── 01_component_architecture.*    ← What talks to what
│   ├── 02_system_architecture.*       ← Layered deployment view
│   └── 03_breadboard_tinkercad.*      ← Photorealistic breadboard layout
└── firmware/                          ← ESP-IDF firmware source (MVP-1)
    ├── README.md                      ← Firmware-specific build instructions
    ├── platformio.ini                 ← ESP32 NodeMCU-32S, ESP-IDF framework
    ├── partitions.csv                 ← Custom flash layout (24 KB NVS)
    ├── CMakeLists.txt                 ← ESP-IDF project root
    ├── include/
    │   └── pinmap.h                   ← Single source of truth for all pins
    └── src/
        ├── CMakeLists.txt             ← Component registration
        ├── main.c                     ← app_main + FreeRTOS tasks
        ├── mpu6050.c / mpu6050.h      ← I2C motion sensor driver
        ├── ds18b20.c / ds18b20.h      ← 1-Wire temperature driver
        ├── battery.c / battery.h      ← ADC battery voltage monitor
        ├── ble_service.c / ble_service.h  ← Bluedroid GATT server
        └── activity_score.c / activity_score.h ← Activity scoring (MVP-1 naive)
```

## Quick Start — MVP-1

### Hardware

1. Buy components from the [BOM](docs/01_PROJECT_OVERVIEW.md#6-hardware--bill-of-materials-per-band) (~₹1,178 per band)
2. Wire on breadboard using the [breadboard diagram](diagrams/03_breadboard_tinkercad.png)
3. Follow the [wiring map](docs/02_SYSTEM_ARCHITECTURE.md#5-wiring-map) for pin-by-pin connections

### Firmware

```bash
# 1. Install VS Code + PlatformIO IDE extension
# 2. Open firmware/ as a PlatformIO project
# 3. Connect ESP32 via USB

pio run                    # Build
pio run --target upload    # Flash
pio device monitor         # Monitor (115200 baud)
```

### Verify

1. Serial monitor shows JSON every 30 seconds:
   ```json
   {"t":30,"motion":0.018,"temp":27.43,"batt":3.92,"score":0}
   ```
2. Install **nRF Connect** on your phone
3. Scan for `GoatBand-001`, connect, subscribe to notifications
4. Shake breadboard → `motion` and `score` jump
5. Pinch temp probe → `temp` rises toward body temperature
6. Multimeter check → `batt` matches actual voltage within 0.05V

## Hardware Pin Map

| ESP32 GPIO | Function | Connected To |
|------------|----------|-------------|
| 21 | I2C SDA | MPU-6050 SDA |
| 22 | I2C SCL | MPU-6050 SCL |
| 4 | 1-Wire | DS18B20 DATA + 4.7kΩ pullup |
| 35 | ADC1_CH7 | Battery divider midpoint |
| 23 | SPI MOSI | LoRa SX1276 (MVP-4) |
| 19 | SPI MISO | LoRa SX1276 (MVP-4) |
| 18 | SPI SCK | LoRa SX1276 (MVP-4) |
| 5 | SPI CS | LoRa SX1276 (MVP-4) |
| 14 | Reset | LoRa SX1276 (MVP-4) |
| 26 | DIO0 | LoRa SX1276 (MVP-4) |

## Configuration Locked for This Plan

| Decision | Choice |
|----------|--------|
| Radio | BLE + LoRa (BLE for phone, LoRa for shed range) |
| Power | Battery-only with TP4056 in-place charging |
| Sensors | MPU-6050 motion + DS18B20 temperature |
| Test scale | 5 bands on real goats |
| Framework | Pure ESP-IDF, no Arduino |
| Budget | ~₹15,000 for 5 bands + tools + Pi hub |

## Documentation Index

| Document | Description |
|----------|-------------|
| [Project Overview](docs/01_PROJECT_OVERVIEW.md) | Problem, solution, BOM, 5 MVPs, build sequence |
| [System Architecture](docs/02_SYSTEM_ARCHITECTURE.md) | Layers, component diagrams, wiring map, breadboard |
| [Firmware Design](docs/03_FIRMWARE_DESIGN.md) | FreeRTOS tasks, boot sequence, concurrency, memory |
| [Module Reference](docs/04_MODULE_REFERENCE.md) | Per-module API, data structures, sequence diagrams |
| [Data Flow & Protocols](docs/05_DATA_FLOW_AND_PROTOCOLS.md) | I2C, 1-Wire, BLE specs, data pipeline, alert logic |
| [Build & Deploy](docs/06_BUILD_AND_DEPLOYMENT.md) | Build, flash, test, checklists, risks, troubleshoot |

## License

This project is licensed under the MIT License.
