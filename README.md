# ClassVitals — RFID-Gated Multi-Sensor Smart Classroom System

> Arduino Uno project | CMR University, SoET — Dept. of ECE | 2026–27

---

## What It Does

ClassVitals makes the **RFID card tap the power gate** of a classroom.

- **Room empty** (count = 0): fan, buzzer and light are forced **OFF**, no matter what any sensor reads.
- **Room occupied** (count ≥ 1): sensors take over — fan above 28 °C, buzzer above sound reading 600, light below LDR reading 300.
- Every entry and exit is **logged on the Serial Monitor** with a timestamp and duration, giving attendance as a free by-product of the occupancy gate.
- A **20×4 I2C LCD** displays live temperature, humidity, light level, sound level and output states.

---

## Team

| Name | USN |
|---|---|
| Chetan Venugopala | 25BBTCS327 |
| Aditya | 25BBTCS319 |
| Chilaka Hrushikesh | 25BBTCS328 |
| Ratan Bahadur Mohanta | 25BBTCS347 |

**Guide:** Prof. Akshatha Bhat, Dept. of ECE, SoET, CMRU, Bangalore.

---

## Hardware Components

| # | Component | Specification |
|---|---|---|
| 1 | Arduino Uno R3 | ATmega328P |
| 2 | RFID Module | MFRC522, 13.56 MHz, SPI |
| 3 | RFID Cards | Mifare S50, 13.56 MHz — 5 cards |
| 4 | Temperature & Humidity Sensor | DHT11, 3-pin module |
| 5 | Sound Sensor | LM393 mic module, analog out |
| 6 | Light Sensor | LDR with LM393 comparator, analog out |
| 7 | LCD Display | 20×4 with I2C backpack (address 0x27 or 0x3F) |
| 8 | Relay Module | 5 V, 1 channel (active LOW) |
| 9 | DC Motor (Fan) | 5 V small motor on its own supply via relay |
| 10 | Buzzer | Active piezo, 5 V |
| 11 | LED | 5 mm with 220 Ω resistor |
| 12 | Breadboard + Jumper Wires | 400-point half-size |

> ⚠️ **MFRC522 runs at 3.3 V only. Never connect its VCC to the Uno 5 V pin.**

---

## Pin Map

| Pin | Connected To | Type |
|---|---|---|
| D2 | Push button (test, active LOW) | Input, internal pull-up |
| D4 | Buzzer + | Digital output |
| D5 | LED + through 220 Ω | Digital output |
| D6 | Relay IN (fan) | Digital output |
| D9 | MFRC522 RST | Digital output |
| D10 | MFRC522 SDA (SS) | SPI chip select |
| D11 | MFRC522 MOSI | SPI |
| D12 | MFRC522 MISO | SPI |
| D13 | MFRC522 SCK | SPI |
| A0 | LDR module AO | Analog input |
| A1 | DHT11 DATA | Digital input |
| A2 | Sound sensor AO | Analog input |
| A4 | LCD I2C SDA | I2C |
| A5 | LCD I2C SCL | I2C |

---

## Thresholds

| Sensor | ON Condition | OFF Condition |
|---|---|---|
| Fan | Temperature > 28 °C | < 27 °C (1 °C hysteresis) |
| Buzzer | Sound peak > 600 (raw, 30 ms window) | ≤ 600 |
| Light (LED) | LDR reading > 350 (dark) | < 320 (bright) |

All thresholds are **only active when at least one student is inside**.

---

## Libraries Required

Install all three from the Arduino Library Manager:

- **DHT sensor library** — Adafruit (also install Adafruit Unified Sensor)
- **LiquidCrystal I2C** — Frank de Brabander
- **MFRC522** — GithubCommunity

---

## How to Set Up

1. Install the three libraries above.
2. Open `ClassVitals.ino` in the Arduino IDE.
3. Find the `students[]` array near the top and **replace the UIDs** with your actual card UIDs. Scan each card with the serial monitor to find its UID.
4. If your LDR reads **high** when dark (LED switches on in bright light), set `LDR_DARK_IS_LOW` to `false`.
5. If the LCD stays blank, change `0x27` to `0x3F` in the `LiquidCrystal_I2C lcd(...)` line.
6. Select **Arduino Uno** as the board, choose the correct COM port, and upload.
7. Open Serial Monitor at **9600 baud** to see live entry/exit logs and sensor readings.

---

## Test Button

A push button on **D2** (other leg to GND) simulates a card tap for Chetan's UID (`8C29F506`) without needing a physical card scan. First press = entry, second press = exit.

---

## LCD Layout

```
CLASSVITALS
Temp:28.5°C Fan:ON
Light:320 LED:OFF
Sound:412 Buzz:OFF
```

When the room is empty, Line 4 reads: `ROOM EMPTY: ALL OFF`

---

## Serial Monitor Sample Output

```
CLASSVITALS ready.
RFID UID: 8C29F506
ENTRY: Chetan | UID: 8C29F506
Temperature: 29.0 C | Humidity: 66.0 % | LDR: 320 | Sound: 210
EXIT: Chetan | Duration: 325 seconds
```

---

## Project Status

| Item | Status |
|---|---|
| Requirement survey (n = 50) | ✅ Done |
| Circuit and wiring | ✅ Done |
| Firmware with room gate + LCD | ✅ Done |
| Breadboard assembly and testing | ✅ Done |
| Six-zone PIR extension | 🔲 Planned |

---

## Future Scope

- Six-zone PIR extension using two PCF8574 I2C port expanders and an 8-channel relay board
- EEPROM storage to preserve occupancy count across resets
- Wi-Fi logging via ESP32 / ESP8266

---

## References

- NXP MFRC522 Datasheet
- Aosong DHT11 Datasheet
- Microchip ATmega328P Datasheet
- IEEE Xplore 7991249 — PIR-based energy saving
- IEEE Xplore 6353853 — RFID + PIR lighting control
- Wokwi Simulator: https://wokwi.com
- Tinkercad: https://www.tinkercad.com

---

*CMR University, School of Engineering and Technology — Dept. of CSE — 2026–27*
