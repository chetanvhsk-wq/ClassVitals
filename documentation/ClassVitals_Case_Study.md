# ClassVitals — Technical Case Study and Project Report

> **Project type:** Embedded systems / smart classroom monitoring  
> **Platform:** Arduino Uno R3 (ATmega328P)  
> **Core functions:** RFID-based student entry/exit tracking, room-occupancy gating, environmental sensing, and automatic output control  
> **Repository:** [chetanvhsk-wq/ClassVitals](https://github.com/chetanvhsk-wq/ClassVitals)  
> **Primary implementation reviewed:** `ClassVitals.ino` and `README.md` on the `main` branch  
> **Prepared:** 19 November 2026

---

## 1. Executive Summary

ClassVitals is an embedded smart-classroom prototype that combines an RFID-based attendance mechanism with environmental monitoring and occupancy-aware control of classroom devices. A student presents a registered RFID card to an MFRC522 reader. The firmware identifies the card UID, alternates the student's state between entry and exit, updates an in-memory occupancy count, and prints attendance events to the Serial Monitor.

The occupancy count also acts as a control gate. When the room is empty, the firmware forces the fan output off. When at least one registered student is inside, temperature, light, and sound readings are used to control a fan, LED, and buzzer. A 20×4 I2C LCD presents live sensor readings and output states.

The project demonstrates how low-cost microcontrollers, sensors, and RFID can be combined into a classroom automation prototype. It is a prototype rather than a complete institutional attendance platform: the reviewed firmware stores attendance state in RAM, uses a hard-coded list of five sample UIDs, and does not include a persistent database, network dashboard, or verified real-world accuracy study.

## 2. Problem Statement

Conventional classroom attendance and room-device operation are often separate activities. Attendance may be recorded manually, while lights, fans, and alerts are controlled independently. This can lead to repetitive manual work and devices remaining active when a classroom is unoccupied.

ClassVitals explores an integrated approach in which an RFID card tap records a student's entry or exit and the resulting occupancy state informs selected classroom automation decisions.

### 2.1 Problem addressed

- Manual attendance recording can consume class time.
- Classroom occupancy may not be reflected in device-control decisions.
- Environmental conditions such as temperature, light, and sound may require attention during class.
- A compact, low-cost demonstrator is useful for learning embedded systems integration.

### 2.2 Proposed solution

Use an Arduino Uno as the central controller. An MFRC522 reader captures card UIDs; a DHT11, LDR module, and sound sensor provide environmental measurements; and the controller updates a fan relay, LED, buzzer, LCD, and Serial Monitor.

## 3. Project Objectives

1. Identify registered RFID cards using the MFRC522 module.
2. Record alternating entry and exit events for each registered card.
3. Maintain a live count of registered students currently marked inside.
4. Prevent the fan from running when the occupancy count is zero.
5. Switch the fan based on temperature when the room is occupied.
6. Switch an LED according to the configured light threshold.
7. Activate a buzzer melody when the measured sound peak exceeds its configured threshold.
8. Display sensor readings and output states on a 20×4 I2C LCD.
9. Provide Serial Monitor messages for RFID detection, attendance events, and sensor readings.

## 4. Scope and Assumptions

This report describes the implementation visible in the repository at the time of review. The project is a classroom-scale prototype.

**Included in the reviewed implementation**
- Arduino Uno firmware.
- MFRC522 RFID card reading.
- Five preconfigured student UID entries in the source.
- Per-student in-memory inside/outside state.
- Occupancy counter.
- DHT11 temperature and humidity readings.
- Analog light and sound readings.
- Fan, LED, buzzer, LCD, and Serial Monitor output.

**Not evidenced by the reviewed source**
- A cloud database or web dashboard.
- Persistent attendance history across power cycles.
- Date-wise or subject-wise attendance percentage calculation.
- Automatic time synchronization or calendar timestamps.
- A production-grade authentication or anti-spoofing mechanism.
- A deployed, independently validated installation at scale.

## 5. System Overview

### 5.1 High-level architecture

```text
       Registered RFID Card
                |
                v
        MFRC522 RFID Reader
                |
                v
          Arduino Uno
          /    |     \
         /     |      \
        v      v       v
   DHT11     LDR     Sound Sensor
 (Temp/RH)  (Light)  (Analog peak)
        \      |      /
         \     |     /
          v    v    v
       Control and Attendance Logic
          /       |        \
         v        v         v
      Fan relay   LED     Buzzer
          \       |        /
           \      |       /
             20x4 I2C LCD
                  |
             Serial Monitor
```

The RFID path updates student state and room occupancy. The sensor path updates measurements and output decisions. The LCD and Serial Monitor provide local feedback.

### 5.2 Operational principle

1. The firmware initializes the LCD, DHT11, SPI bus, and MFRC522 reader.
2. The RFID reader checks for a new card.
3. The card UID is converted to an uppercase hexadecimal string.
4. The UID is matched against the `students[]` array.
5. If the card is registered and not currently marked inside, it is recorded as an entry.
6. If the same registered student is already marked inside, the next accepted tap records an exit.
7. The global occupancy count is incremented or decremented.
8. The output-control routine applies the fan, LED, and buzzer rules.
9. The LCD is refreshed and relevant messages are printed to the Serial Monitor.

## 6. Hardware Components

| Component | Role in the system | Interface / notes |
|---|---|---|
| Arduino Uno R3 | Main controller | ATmega328P |
| MFRC522 RFID reader | Reads RFID card UIDs | SPI; 3.3 V module |
| RFID cards/tags | Student identification tokens | Compatible 13.56 MHz cards |
| DHT11 | Temperature and humidity sensing | Digital data pin |
| LDR module | Ambient light measurement | Analog output |
| LM393 sound sensor module | Sound-level proxy measurement | Analog output |
| 20×4 LCD with I2C backpack | Local display | I2C; source uses address 0x27 |
| 5 V relay module | Fan switching interface | Source drives relay control from D6 |
| DC fan / motor | Demonstration of ventilation control | Requires suitable external motor supply and driver/relay |
| LED and 220 Ω resistor | Light output indicator | D5 |
| Active/passive buzzer | Sound alert | D4 |
| Push button | Test input | D2, active LOW |
| Breadboard and jumper wires | Circuit assembly | Prototyping |

**Electrical safety note:** The MFRC522 module is a 3.3 V device. Do not connect its VCC pin to the Arduino Uno 5 V supply. The Arduino output pin is a control signal and must not power a DC fan directly. Use an appropriately rated relay or driver and an external supply for the motor, with wiring and grounding appropriate to the selected circuit.

## 7. Pin Configuration

The following pin assignments are taken from the reviewed firmware.

| Arduino pin | Connected component | Function |
|---|---|---|
| D2 | Push button | Test attendance tap; INPUT_PULLUP |
| D4 | Buzzer | Digital output |
| D5 | LED | Digital output |
| D6 | Fan relay input | Digital output |
| D7 | Secondary fan control | Held LOW by the firmware |
| D9 | MFRC522 RST | RFID reset |
| D10 | MFRC522 SDA / SS | SPI chip select |
| D11 | MFRC522 MOSI | SPI data |
| D12 | MFRC522 MISO | SPI data |
| D13 | MFRC522 SCK | SPI clock |
| A0 | LDR module AO | Analog light input |
| A1 | DHT11 DATA | Sensor data |
| A2 | Sound module AO | Analog sound input |
| A4 | LCD SDA | I2C data |
| A5 | LCD SCL | I2C clock |

The source configures the LCD at I2C address `0x27`. Some LCD backpacks use a different address; verify the actual module address before assuming the display is faulty.

## 8. Software and Libraries

The firmware is written in Arduino C++.

| Library | Purpose |
|---|---|
| `Wire.h` | I2C communication with LCD |
| `SPI.h` | SPI communication with MFRC522 |
| `MFRC522.h` | RFID reader interface |
| `LiquidCrystal_I2C.h` | 20×4 I2C LCD interface |
| `DHT.h` | DHT11 temperature and humidity interface |

The README instructs users to install the Adafruit DHT sensor library (and Adafruit Unified Sensor dependency), LiquidCrystal I2C, and MFRC522 through Arduino Library Manager.

## 9. Firmware Design and Logic

### 9.1 Student data structure

Each student record contains:
- RFID UID string.
- Student display name.
- Boolean `inside` state.
- Entry time measured using the Arduino `millis()` clock, converted to seconds.

The source currently contains five sample records. These values must be replaced with the actual card UIDs and student names before deployment.

### 9.2 RFID attendance processing

The `checkRFID()` function checks whether a new card is present, reads its serial data, formats the UID as uppercase hexadecimal, and calls `handleCard()`.

The `handleCard()` function:
- Applies a same-card debounce window of 2,000 milliseconds.
- Looks up the UID in the registered student array.
- Prints an unknown-card message and ignores UIDs that are not registered.
- Changes the student's state from outside to inside on entry.
- Stores the current elapsed uptime in seconds as the entry time.
- Increments `insideCount`.
- On the next accepted tap for that student, changes the state to outside.
- Decrements `insideCount` if it is greater than zero.
- Prints the elapsed session duration in seconds.

The design is a per-card toggle: it does not infer direction from separate entry and exit readers. It assumes a student's next accepted tap means the opposite state.

### 9.3 Occupancy counter

`insideCount` represents the number of registered student records currently marked inside.

```text
On accepted ENTRY:
    student.inside = true
    insideCount = insideCount + 1

On accepted EXIT:
    student.inside = false
    insideCount = max(insideCount - 1, 0)
```

The fan logic uses this counter as an occupancy gate. The source does not apply this same occupancy gate to the LED or buzzer.

### 9.4 Temperature and fan logic

The firmware uses:
- Fan ON threshold: temperature greater than 28 °C.
- Fan OFF threshold: temperature below 27 °C.
- Occupancy requirement: at least one registered student marked inside.

The 1 °C gap is hysteresis. It reduces rapid switching when the temperature fluctuates near the threshold.

```text
If insideCount == 0:
    Fan OFF
Else if temperature > 28 °C:
    Fan ON
Else if fan is ON and temperature < 27 °C:
    Fan OFF
Otherwise:
    Keep the previous fan state
```

The D6 Arduino pin controls the relay input in the source; it does not supply fan motor current.

### 9.5 Light and LED logic

The code treats a higher LDR analog reading as darkness:
- LED turns ON when the reading rises above 350.
- LED turns OFF when the reading falls below 320.
- The gap between 350 and 320 provides hysteresis.

The correct polarity depends on the particular LDR module and its wiring. The module should be tested under bright and dark conditions and the threshold/polarity adjusted accordingly.

### 9.6 Sound and buzzer logic

The firmware samples the sound sensor for a 30 ms window and stores the highest analog reading observed in that interval. The buzzer is requested when the peak exceeds 600.

The source contains a repeating sequence of tones for the buzzer. This is a simple threshold-based sound response, not calibrated decibel measurement or speech recognition.

### 9.7 LCD display

The 20×4 LCD is refreshed approximately every 250 ms and presents:
- Line 1: Project name.
- Line 2: Temperature and fan state.
- Line 3: Light reading and LED state.
- Line 4: Sound reading and buzzer state.

The current reviewed firmware does not show an occupancy count or student name on the LCD.

### 9.8 Serial Monitor

The firmware starts Serial communication at 9,600 baud. It prints:
- MFRC522 version and initialization result.
- Detected card UID.
- Entry and exit event messages.
- Session duration on exit.
- Temperature and humidity.
- LDR and sound readings.
- Test-button events.

The timestamp/duration value is based on elapsed time since the microcontroller booted. It is not a real clock time or date/time stamp.

## 10. Configured Thresholds

| Input | Configured condition | Firmware response |
|---|---|---|
| Temperature | Above 28 °C while occupied | Fan ON |
| Temperature | Below 27 °C while fan is ON | Fan OFF |
| LDR analog value | Above 350 | LED ON |
| LDR analog value | Below 320 while LED is ON | LED OFF |
| Sound peak | Above 600 | Buzzer melody requested |
| Occupancy | Count equals zero | Fan forced OFF |

These are source-code constants, not measured or scientifically calibrated environmental limits. Thresholds should be tuned to the particular sensors, installation, and classroom conditions.

## 11. User Interaction and Operating Procedure

### 11.1 Initial setup

1. Assemble the circuit using the pin map in this report.
2. Verify the MFRC522 is powered from 3.3 V.
3. Install the required Arduino libraries.
4. Open `ClassVitals.ino` in Arduino IDE.
5. Replace the sample RFID UIDs and student names with the actual values.
6. Verify the LDR polarity and adjust the threshold logic if required.
7. Confirm the LCD I2C address.
8. Select Arduino Uno and the correct serial port.
9. Upload the firmware.
10. Open Serial Monitor at 9,600 baud.

### 11.2 Attendance operation

1. A registered student taps their card.
2. The system logs ENTRY and increments the occupancy count.
3. That student taps again when leaving.
4. The system logs EXIT, prints elapsed duration, and decrements the count.
5. The same card is ignored if read again inside the 2-second debounce interval.

### 11.3 Test button

The D2 push button simulates a tap for the hard-coded UID `8C29F506`. It is intended for bench testing without presenting the physical card. It does not test arbitrary student cards.

## 12. Representative Output

The following is an illustrative example based on the output format in the source and README; it is not a claim that these exact readings were recorded in a controlled test.

```text
CLASSVITALS ready.
RC522 firmware: 0x92
RFID communication OK
RFID UID: 8C29F506
ENTRY: Chetan | UID: 8C29F506
Temperature: 29.0 C | Humidity: 66.0 % | LDR: 320 | Sound: 210
EXIT: Chetan | Duration: 325 seconds
```

## 13. Testing and Validation Plan

The repository README describes the circuit and firmware as assembled/tested, but no structured test dataset, test log, accuracy percentage, or controlled measurement report was present in the reviewed source. The following matrix is therefore a recommended validation plan, not a list of completed tests.

| Test ID | Test procedure | Expected result | Evidence to record |
|---|---|---|---|
| T01 | Power the board and open Serial Monitor | Startup message appears | Serial log |
| T02 | Present a registered card | UID and ENTRY appear | UID and event log |
| T03 | Present the same card after debounce period | EXIT appears | Event log and duration |
| T04 | Present an unregistered card | Unknown UID message; count unchanged | Serial log |
| T05 | Present the same card twice within 2 seconds | Second read is ignored | Serial log |
| T06 | Keep occupancy at zero and raise temperature | Fan remains OFF | Temperature and relay state |
| T07 | Register a student inside and raise temperature above threshold | Fan relay output turns ON | Temperature and output measurement |
| T08 | While fan is ON, cool below 27 °C | Fan relay output turns OFF | Temperature and output measurement |
| T09 | Change LDR reading above/below thresholds | LED responds with hysteresis | ADC readings and LED state |
| T10 | Raise sound peak above threshold | Buzzer melody is requested | Sound reading and buzzer response |
| T11 | Inspect LCD during operation | Four display rows show current values/states | LCD photograph |
| T12 | Restart the Arduino after an entry | State resets to initial RAM values | Before/after serial log |

### 13.1 Suggested performance measures

For a formal case study, record the following using repeated trials:
- RFID read success rate = successful reads / total card presentations.
- Attendance event correctness = correctly classified entry/exit events / accepted registered-card reads.
- False-read rate = unintended accepted reads / total observed reads.
- Sensor response delay = time between crossing a threshold and output change.
- LCD refresh stability = number of observed display glitches over a timed run.
- Fan control correctness = number of correct fan decisions / total fan test cases.

Do not insert numerical results until actual trials have been completed and documented.

## 14. Results and Observations Supported by the Source

The reviewed implementation establishes the following software design facts:
- The program supports up to five entries in its current hard-coded student table.
- A registered card alternates between inside and outside states.
- The same UID is suppressed for 2 seconds after a read.
- Entry and exit events are printed to Serial Monitor.
- Exit events include elapsed session duration in seconds.
- The fan is gated by the occupancy counter.
- Temperature control uses 28 °C ON and 27 °C OFF thresholds.
- The LED uses a light threshold with hysteresis.
- The buzzer is requested when a 30 ms sound peak exceeds 600.
- The LCD is configured as a 20×4 I2C display.

These facts describe the source code. They should not be interpreted as independent proof of field reliability, attendance accuracy, or energy savings.

## 15. Limitations and Risks

1. **Volatile state:** Student inside/outside flags and the occupancy count are stored in RAM. They reset when the Arduino restarts or loses power.
2. **No persistent attendance log:** The source prints events to Serial Monitor but does not write attendance records to EEPROM, SD card, or a server.
3. **Small hard-coded list:** The current source contains five sample students. UID and name registration requires editing and uploading firmware.
4. **Toggle assumption:** The next accepted tap is interpreted as the opposite state. A missed tap can desynchronize the student's logical state from their physical location.
5. **No real date/time:** Session timing uses `millis()`, which measures uptime. It cannot provide a calendar timestamp.
6. **RFID UID is not strong identity proof:** A UID-based allowlist is a basic prototype mechanism and should not be treated as secure authentication.
7. **Sensor calibration:** LDR and sound thresholds are raw analog values and depend on the modules, wiring, supply, and environment.
8. **Sound measurement:** A 30 ms analog peak and threshold do not represent calibrated sound pressure level in dB.
9. **Fan hardware dependency:** Safe and reliable fan operation depends on correct relay/driver wiring and an adequate external power source.
10. **Partial occupancy gating:** In the reviewed code, the fan is occupancy-gated; the LED and buzzer are not explicitly forced off when the room is empty.
11. **No structured validation report:** Numerical test results, sample size, and statistical analysis must be collected before claiming accuracy or energy savings.
12. **No network interface:** The reviewed source does not provide remote monitoring, cloud synchronization, or a mobile application.

## 16. Future Enhancements

### 16.1 Attendance and data management
- Add EEPROM or SD-card logging for power-cycle persistence.
- Add an RTC module for date/time-stamped events.
- Add exportable CSV attendance records.
- Add configurable student registration without recompiling firmware.
- Add attendance summaries and reporting.

### 16.2 Connectivity
- Add an ESP32 or ESP8266 communication layer.
- Send records to a backend over a secured connection.
- Build a teacher dashboard for current occupancy and attendance reports.
- Include offline queuing and retry logic for network interruptions.

### 16.3 Hardware and sensing
- Add multiple occupancy zones using PIR sensors or suitable presence sensors.
- Use calibrated temperature/light/sound measurement methods.
- Add electrical isolation and suitable protection for switched loads.
- Add a manual override and fail-safe behavior for classroom devices.

### 16.4 Reliability and security
- Use explicit entry and exit controls or direction sensing to avoid toggle ambiguity.
- Validate duplicate-card behavior and simultaneous card scans.
- Protect attendance data and restrict administrative access.
- Test power recovery, sensor faults, and RFID reader disconnection.
- Avoid relying on RFID UID alone for high-assurance identity verification.

## 17. Conclusion

ClassVitals demonstrates an embedded systems approach to combining RFID-based attendance events with classroom environmental monitoring. The Arduino Uno coordinates the MFRC522 reader, DHT11, LDR module, sound sensor, relay-controlled fan, LED, buzzer, and LCD. The occupancy counter provides the central link between attendance state and fan operation.

The prototype is suitable as an educational demonstration of microcontroller programming, sensor integration, RFID communication, threshold-based automation, and local status reporting. To evolve into a dependable deployment, it needs persistent attendance storage, date/time support, a more robust entry/exit method, configurable registration, calibrated sensors, electrical validation, and a documented test campaign.

## 18. References and Project Materials

### Repository materials
- [ClassVitals repository](https://github.com/chetanvhsk-wq/ClassVitals)
- [Firmware — ClassVitals.ino](https://github.com/chetanvhsk-wq/ClassVitals/blob/main/ClassVitals.ino)
- [Project README](https://github.com/chetanvhsk-wq/ClassVitals/blob/main/README.md)
- [Project report PDF in repository](https://github.com/chetanvhsk-wq/ClassVitals/blob/main/documentation/ClassVitals_Report_Final_v8.pdf)
- [Project presentation in repository](https://github.com/chetanvhsk-wq/ClassVitals/blob/main/documentation/ClassVitals_Presentation%20main.pptx)
- [Wiring plan image](https://github.com/chetanvhsk-wq/ClassVitals/tree/main/documentation/img)

### Technical references to consult
- MFRC522 / MIFARE reader datasheet and module documentation.
- Arduino Uno Rev3 technical specifications.
- DHT11 sensor datasheet.
- ATmega328P datasheet.
- Datasheets for the exact relay module, sound sensor, and LDR module used in the build.

> Add the exact datasheet URLs and access dates required by your institution's citation style before submitting a formal academic report.

---

## Appendix A — Source Configuration Snapshot

| Parameter | Value in reviewed firmware |
|---|---:|
| Serial baud rate | 9,600 |
| Student entries in array | 5 |
| Duplicate-card suppression window | 2,000 ms |
| Sensor interval | 2,000 ms |
| LCD refresh interval | 250 ms |
| Sound sampling window | 30 ms |
| Fan ON threshold | > 28 °C |
| Fan OFF threshold | < 27 °C |
| Light ON threshold | > 350 |
| Light OFF threshold | < 320 |
| Sound threshold | > 600 |
| LCD I2C address | 0x27 |

## Appendix B — Suggested Evidence to Attach

For a complete academic submission, attach genuine project evidence:
1. Photograph of the fully assembled circuit.
2. Close-up photograph of the Arduino, MFRC522, and sensor wiring.
3. Screenshot of Serial Monitor showing a registered entry and exit.
4. Photograph of LCD readings during operation.
5. Fan/relay test photographs showing both OFF and ON states.
6. A table of repeated RFID and threshold tests with date, conditions, expected result, and observed result.
7. A block diagram and circuit schematic matching the final physical wiring.

*End of report.*
