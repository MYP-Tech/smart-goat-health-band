# GoatBand — Project Overview

## 1. Introduction

GoatBand is an IoT-based smart neckband system designed for early illness detection in commercial goat farms. The system continuously monitors goat activity levels and body temperature using on-animal wearable devices, transmits telemetry data to a centralized hub, and delivers actionable health alerts to farmers via a mobile application.

The project addresses a critical challenge in livestock management: detecting illness in goats before visible symptoms appear, enabling timely veterinary intervention and reducing mortality rates in commercial herds.

## 2. Problem Statement

Commercial goat farming operations suffer significant financial losses due to late detection of illness. By the time a goat shows visible signs of sickness (lethargy, refusal to eat, isolation from the herd), the disease may have progressed to a stage where treatment is less effective or other animals have already been exposed.

Key challenges this project solves:

- **Delayed detection**: Manual observation by farmers fails to catch early behavioral changes
- **Scale limitations**: A single farmer cannot continuously monitor dozens or hundreds of goats
- **Lack of objective data**: Subjective visual inspection is unreliable for early-stage illness
- **No historical baselines**: Without per-animal data, subtle deviations go unnoticed

## 3. Solution Architecture (High Level)

GoatBand deploys a multi-layer IoT architecture:

| Layer | Component | Role |
|-------|-----------|------|
| Layer 1 — Edge | ESP32 neckband | Sensors + edge compute on the goat |
| Layer 2 — Gateway | Raspberry Pi hub | Aggregates data from all bands in a shed |
| Layer 3 — Cloud | Backend services | Stores history, runs alert engine, sends notifications |
| Layer 4 — Client | Flutter mobile app | Farmer-facing dashboard, alerts, and per-goat profiles |

## 4. Hardware Bill of Materials (MVP-1)

| Component | Specification | Purpose | Unit Cost (INR) |
|-----------|---------------|---------|-----------------|
| ESP32 NodeMCU-32S | 240 MHz dual-core, WiFi + BLE | Main MCU | ~₹350 |
| MPU-6050 | 6-axis IMU (accelerometer + gyroscope) | Motion sensing | ~₹120 |
| DS18B20 | Waterproof 1-Wire temperature probe | Body temperature | ~₹80 |
| TP4056 module | Li-ion charge controller | Battery charging | ~₹30 |
| 18650 Li-ion cell | 3.7V, 2600–3000 mAh | Power supply | ~₹150 |
| SX1276 LoRa module | 433 MHz, long-range radio (MVP-4) | Shed-range comms | ~₹250 |
| Passive components | Resistors, capacitors, pullups | Signal conditioning | ~₹20 |
| **Total per band** | | | **~₹1,178** |

**Target deployment**: 5 bands + tools + Raspberry Pi hub ≈ ₹15,000 total budget.

## 5. MVP Roadmap

The project follows a 5-stage Minimum Viable Product development plan spanning 12 weeks:

### MVP-1: Bench Prototype (Weeks 1–2)
- Assemble breadboard circuit with ESP32 + MPU-6050 + DS18B20
- Flash firmware, confirm sensor readings on serial monitor
- Verify BLE advertising and characteristic subscription
- **Deliverable**: Working bench unit streaming JSON telemetry

### MVP-2: Single Goat Trial (Weeks 3–4)
- Mount one neckband on a live goat for 72 hours
- Add deep sleep between sampling windows to optimize power
- Collect real-world motion and temperature profiles
- **Deliverable**: 72-hour continuous data capture

### MVP-3: Baseline Learning (Weeks 5–7)
- Implement per-goat baseline: 5-day learning of hourly mean/stdev
- Replace naive `activity_score` with baseline deviation detection
- Store baselines in NVS flash
- Alert logic: 3 consecutive low-activity windows → WARNING; 5 + temp > 40°C → CRITICAL
- **Deliverable**: Intelligent per-goat alerting

### MVP-4: LoRa Hub (Weeks 8–10)
- Enable LoRa transmission from bands to shed hub
- Build Raspberry Pi hub with Python daemon
- SQLite buffer for offline resilience
- MQTT uplink to cloud backend
- **Deliverable**: Multi-band farm-scale data collection

### MVP-5: Mobile App + Cloud (Weeks 11–12)
- Deploy cloud backend (FastAPI + TimescaleDB + alert engine)
- Build Flutter mobile app with dashboard, alerts, and goat profiles
- Push notifications via FCM + SMS
- OTA firmware updates
- **Deliverable**: End-to-end production-ready system

## 6. Technology Stack Summary

| Domain | Technology | Rationale |
|--------|-----------|-----------|
| Firmware framework | ESP-IDF v5.x (native) | Direct FreeRTOS access, smaller binary, finer power control |
| Build system | PlatformIO + CMake | Reproducible builds, IDE integration |
| MCU | ESP32 NodeMCU-32S | BLE + WiFi built-in, dual-core for concurrent tasks |
| Motion sensor | MPU-6050 via I2C | Inexpensive 6-axis IMU, well-documented |
| Temperature sensor | DS18B20 via 1-Wire | Waterproof, accurate, minimal wiring |
| Short-range radio | BLE (Bluedroid stack) | Phone pairing for debugging and MVP |
| Long-range radio | LoRa SX1276 at 433 MHz | Shed-range, low-power, license-free band |
| Gateway compute | Raspberry Pi Zero 2W | Small, affordable, runs Python |
| Gateway database | SQLite | Offline buffer, zero-config |
| Cloud messaging | MQTT over TLS | Lightweight pub/sub, topic-per-goat |
| Cloud API | FastAPI (Python) | Async, auto-docs, fast development |
| Time-series DB | TimescaleDB (PostgreSQL) | Purpose-built for sensor data |
| Mobile app | Flutter | Cross-platform (Android + iOS) |
| Push notifications | FCM + APNs + MSG91 SMS | Multi-channel farmer alerting |

## 7. Design Principles

1. **Edge-first compute**: Activity scoring runs on the ESP32 itself, not in the cloud. Only summarized telemetry packets are transmitted.
2. **Graceful degradation**: Each layer works independently. If the cloud is down, the Pi hub buffers locally. If the Pi is down, the band still logs.
3. **Power efficiency**: The firmware targets 50-day battery life through deep sleep, low sampling duty cycle, and LoRa (not WiFi) for transmission.
4. **Modular firmware**: Each driver (MPU-6050, DS18B20, battery, BLE) is an independent compilation unit with a clean init/read API.
5. **Forward-compatible pinmap**: LoRa SPI pins are defined in `pinmap.h` from day one, even though LoRa is activated in MVP-4.
6. **No Arduino dependency**: Pure ESP-IDF ensures full control over FreeRTOS, NVS, ADC v5 driver, and Bluedroid stack.

## 8. Repository Structure

```
GoatBand_release/
├── README.md                          ← Project root README
├── GoatBand_5MVP_Plan.docx            ← Master plan document
├── docs/                              ← Documentation (you are here)
│   ├── 01_PROJECT_OVERVIEW.md
│   ├── 02_SYSTEM_ARCHITECTURE.md
│   ├── 03_FIRMWARE_DESIGN.md
│   ├── 04_MODULE_REFERENCE.md
│   ├── 05_DATA_FLOW_AND_PROTOCOLS.md
│   └── 06_BUILD_AND_DEPLOYMENT.md
├── diagrams/                          ← Architecture diagrams (SVG + PNG)
│   ├── 01_component_architecture.*
│   ├── 02_system_architecture.*
│   └── 03_breadboard_tinkercad.*
└── firmware/                          ← ESP-IDF firmware source
    ├── README.md
    ├── platformio.ini
    ├── partitions.csv
    ├── CMakeLists.txt
    ├── include/
    │   └── pinmap.h
    └── src/
        ├── CMakeLists.txt
        ├── main.c
        ├── mpu6050.c/.h
        ├── ds18b20.c/.h
        ├── battery.c/.h
        ├── ble_service.c/.h
        └── activity_score.c/.h
```
