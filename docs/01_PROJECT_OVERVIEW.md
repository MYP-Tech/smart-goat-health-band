# GoatBand — Project Overview

## 1. Introduction

GoatBand is a smart neckband for early illness detection in commercial goat farms. It uses on-animal wearable sensors to continuously monitor activity levels and body temperature, detects behavioral deviations through per-goat baseline learning, and alerts farmers before symptoms become visually apparent.

The project follows a 5-MVP incremental development approach — each stage produces data that feeds the next, de-risking hardware, firmware, algorithm, radio, and user experience in sequence.

## 2. Problem Statement

Commercial goat farms suffer losses from late illness detection. By the time a goat shows visible sickness (lethargy, refusal to eat, isolation from herd), the disease has often progressed beyond easy treatment, and other animals may already be exposed.

**Why this matters:**

- Manual observation fails to catch early behavioral changes
- A single farmer cannot continuously monitor dozens of goats
- Subjective visual inspection is unreliable for early-stage illness
- Without per-animal data, subtle activity deviations go unnoticed

## 3. Solution — What GoatBand Does

Each goat wears a neckband containing:

- **MPU-6050 accelerometer** — tracks motion at 20 Hz
- **DS18B20 temperature probe** — reads skin temperature every 30 seconds
- **ESP32 microcontroller** — runs edge compute (activity scoring) on-device
- **BLE radio** — streams JSON telemetry to phone for testing
- **LoRa radio** (from MVP-4) — sends data to a shed hub for farm-scale collection

The band computes an **activity score (0–100)** by comparing current motion to a learned per-goat baseline. If a goat is abnormally inactive for extended periods (especially with elevated temperature), the system fires an alert.

## 4. Why 5 MVPs and Not One Big Build

| Reason | MVP |
|--------|-----|
| Goats chew, scratch, sleep, and rub against fences — none of this is in any spec | MVP-2 discovers real-world edge cases on one tame goat |
| LoRa range claims of 2 km are optimistic in dense farms with metal sheds | MVP-4 measures the real range before committing |
| The activity scoring algorithm is the differentiator — if it's wrong, everything downstream is wrong | MVP-3 validates per-goat baselines actually work |
| Battery life claims need real data | Every MVP measures actual current draw |

## 5. The 5 MVPs

### MVP-1 — Bench Prototype (Week 1–2)

**Goal**: Prove every component works on a breadboard, end-to-end, before any soldering.

| Item | Detail |
|------|--------|
| Units | 1 breadboard build |
| Hardware | ESP32 + MPU-6050 + DS18B20 + TP4056 + 18650 + voltage divider |
| Firmware | Read sensors every 30s, log to serial, BLE characteristic exposes JSON |
| Test | Shake the band by hand, breathe on temp sensor, watch numbers change |
| Pass criteria | Phone connects via BLE, receives packet every 30s, numbers move when you move it |

### MVP-2 — One Goat, 72 Hours (Week 3)

**Goal**: Discover what real goat behavior looks like in raw data, before writing any scoring logic.

| Item | Detail |
|------|--------|
| Units | 1 PCB-soldered band in IP54 enclosure |
| Firmware | Logs raw motion + temp every 10s to onboard flash, syncs to phone over BLE |
| Test | One tame, healthy goat, 72 hours continuous |
| Pass criteria | Continuous logs for 72h, no dropouts, battery survives |
| What you'll find | Sleeping ≠ sick, chewing creates rhythmic motion, sun-bathing spikes temp |

### MVP-3 — 5 Goats, Baseline Learning (Week 4–5)

**Goal**: Validate that per-goat baseline learning works in the real world.

| Item | Detail |
|------|--------|
| Units | 5 PCB bands in proper enclosures |
| Firmware | 5 days baseline learning, then activity score 0–100, still BLE only |
| Test | 5 goats of mixed temperaments (active, lazy, mid) |
| Pass criteria | Each goat's baseline is measurably different, no false alerts on lazy goats |
| What this proves | The core algorithm works. If MVP-3 fails, MVP-4 and MVP-5 are pointless |

### MVP-4 — LoRa Hub + Cloud Sync (Week 6–8)

**Goal**: Validate LoRa range claims and offline-buffering behavior.

| Item | Detail |
|------|--------|
| Units | 5 bands + 1 Raspberry Pi shed hub |
| Hardware | MVP-3 bands + SX1276 LoRa daughterboard, Pi Zero 2W with LoRa hat |
| Test | Walk one band away from hub in 100m increments, note packet loss |
| Pass criteria | 200m reliable through one shed wall, 500m line-of-sight |

### MVP-5 — Mobile App + Farmer Field Test (Week 9–12)

**Goal**: Full loop with a real farmer making decisions from your alerts.

| Item | Detail |
|------|--------|
| Units | 5 bands + hub + mobile app (Flutter) |
| App screens | Dashboard, alerts, individual goat profile, weight log, vaccination log |
| Test | 5 bands on 5 goats, including 1 intentionally under-fed to simulate poor health |
| Pass criteria | Farmer notices the under-fed goat in app before noticing visually |
| What this proves | The product actually changes farmer behavior |

## 6. Hardware — Bill of Materials (Per Band)

| Component | Specific Part | Qty | Cost (₹) |
|-----------|---------------|-----|-----------|
| Microcontroller | ESP32 NodeMCU-32S (38-pin) | 1 | 350 |
| Motion sensor | MPU-6050 GY-521 module | 1 | 80 |
| Temperature sensor | DS18B20 waterproof probe | 1 | 120 |
| Charger module | TP4056 with USB-C + protection | 1 | 50 |
| Battery | 18650 Li-ion 2600 mAh | 1 | 300 |
| Battery holder | 18650 single-cell holder | 1 | 30 |
| Resistor | 4.7kΩ (DS18B20 pullup) | 1 | 1 |
| Resistor pair | 100kΩ (battery divider) | 2 | 2 |
| Slide switch | SPDT mini slide switch | 1 | 10 |
| Perfboard | 5×7cm prototype PCB | 1 | 30 |
| Wire | 30 AWG silicone hookup | 1m | 20 |
| Enclosure | IP54 ABS box ~70×50×25mm | 1 | 120 |
| Strap | 25mm nylon webbing + buckle | 1 | 60 |
| Silica gel | Moisture pack 2g | 1 | 5 |
| LoRa module (MVP-4+) | SX1276 RA-02 433 MHz | 1 | 350 |
| **Per-band total (no LoRa)** | | | **1,178** |
| **Per-band total (with LoRa)** | | | **1,528** |

### Total Project Cost — 5 Bands + Hub

| Item | Qty | Unit (₹) | Total (₹) |
|------|-----|-----------|-----------|
| MVP-1/2/3 bands (no LoRa) | 5 | 1,178 | 5,890 |
| LoRa modules (MVP-4) | 5 | 350 | 1,750 |
| Shed hub (Pi + LoRa hat) | 1 | 3,100 | 3,100 |
| Tools (soldering iron, multimeter, breadboard) | 1 | 2,500 | 2,500 |
| Spare components 20% buffer | — | — | 1,800 |
| **Grand total** | | | **₹15,040** |

## 7. Configuration Locked for This Plan

| Decision | Choice |
|----------|--------|
| Radio | BLE + LoRa (BLE for nearby phone, LoRa for shed range) |
| Power | Battery-only with TP4056 charging in-place (no swap dock for MVP) |
| Sensors | MPU-6050 motion + DS18B20 temperature from MVP-1 |
| Test scale | 5 bands on real goats |
| Firmware framework | Pure ESP-IDF, no Arduino |
| Total budget | ~₹15,000 for 5 bands + tools + Pi hub |

## 8. Build Sequence — Week by Week

| Week | Deliverable | Risk to Watch |
|------|-------------|---------------|
| 1 | Order components, set up PlatformIO, breadboard MVP-1 | Wrong ESP32 variant — buy NodeMCU-32S |
| 2 | MVP-1 firmware running, BLE working, JSON streaming | Library version mismatches |
| 3 | MVP-2 — solder onto perfboard, enclosure, 1 goat 72h | Strap rubs raw — sew a fabric liner |
| 4 | MVP-3 build × 5 units | Soldering quality on small batches |
| 5 | MVP-3 deploy + baseline learning runs | Baselines noisy — extend learning to 7 days |
| 6 | MVP-4 hub build, Pi LoRa receiving | Antenna placement matters |
| 7 | MVP-4 cloud sync + range test | WiFi at the farm — 4G dongle as fallback |
| 8 | MVP-4 review + fix issues | Buffer week |
| 9 | MVP-5 mobile app skeleton | Pick Flutter — single codebase |
| 10 | MVP-5 dashboard + alerts wired | Don't over-design |
| 11 | MVP-5 farmer field test | Farmer has to actually use it |
| 12 | MVP-5 review, write up findings | What broke > what worked |
