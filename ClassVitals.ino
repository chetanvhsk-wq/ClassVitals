/*
  CLASSVITALS - SMART CLASSROOM ATTENDANCE & MONITORING
  Board: Arduino UNO
  LCD: 20x4 I2C

  LCD:
  Line 1: CLASSVITALS
  Line 2: Temperature + Fan status
  Line 3: Light intensity + LED status
  Line 4: Sound reading + Buzzer status
*/

#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

// ---------------- PIN CONFIGURATION ----------------

const byte PIN_BUTTON = 2;
const byte PIN_BUZZER = 4;
const byte PIN_LED = 5;
const byte PIN_FAN = 6;
const byte PIN_FAN2 = 7;

const byte PIN_RST = 9;
const byte PIN_SS = 10;

const byte PIN_LDR = A0;
const byte PIN_DHT = A1;
const byte PIN_SOUND = A2;

// ---------------- HARDWARE ----------------

#define DHTTYPE DHT11

DHT dht(PIN_DHT, DHTTYPE);
MFRC522 rfid(PIN_SS, PIN_RST);
LiquidCrystal_I2C lcd(0x27, 20, 4);

// ---------------- SETTINGS ----------------

const float TEMP_LIMIT = 28.0;
const float TEMP_HYST = 1.0;

// Reversed LDR logic:
// High reading = darkness; LED ON.
// Low reading = bright light; LED OFF.
const int DARK_LIMIT = 350;
const int LIGHT_HYST = 30;

// Sound sensor threshold
const int SOUND_LIMIT = 600;

// Timing
const unsigned long SENSOR_INTERVAL = 2000;
const unsigned long LCD_INTERVAL = 250;
const unsigned long SOUND_WINDOW_MS = 30;
const unsigned long SAME_CARD_MS = 2000;

// ---------------- BUZZER MELODY ----------------
// Original upbeat melody for a passive buzzer.

const int melody[] = {
  784, 784, 659, 784,
  880, 784, 659, 587,
  659, 784, 988, 880,
  784, 659, 587, 523
};

const int noteDuration[] = {
  140, 140, 140, 140,
  220, 140, 140, 220,
  140, 140, 220, 140,
  220, 140, 140, 350
};

const byte melodyLength =
  sizeof(melody) / sizeof(melody[0]);

byte currentNote = 0;
bool playingTune = false;
unsigned long nextNoteTime = 0;

// ---------------- STUDENT REGISTRATION ----------------
// Replace sample UIDs with actual card UIDs.

struct Student {
  const char *uid;
  const char *name;
  bool inside;
  unsigned long entrySeconds;
};

Student students[] = {
  {"8C29F506", "Chetan", false, 0},
  {"6D45FF06", "Student2", false, 0},
  {"00000000", "Student3", false, 0},
  {"11111111", "Student4", false, 0},
  {"22222222", "Student5", false, 0}
};

const byte STUDENT_COUNT =
  sizeof(students) / sizeof(students[0]);

byte insideCount = 0;

// ---------------- SENSOR VALUES ----------------

float temperature = NAN;
float humidity = NAN;

int lightValue = 0;
int soundValue = 0;

// ---------------- OUTPUT STATES ----------------

bool rfidOK = false;
bool fanOn = false;
bool lightOn = false;
bool buzzerOn = false;

// ---------------- TIMERS ----------------

unsigned long lastSensorRead = 0;
unsigned long lastLCDUpdate = 0;
unsigned long lastCardTime = 0;
unsigned long lastButtonTime = 0;

char lastUID[21] = "";

// ==================================================
// SOUND SENSOR
// ==================================================

int readSoundPeak() {
  unsigned long start = millis();
  int peak = 0;

  while (millis() - start < SOUND_WINDOW_MS) {
    int value = analogRead(PIN_SOUND);

    if (value > peak) {
      peak = value;
    }
  }

  return peak;
}

// ==================================================
// READ SENSORS
// ==================================================

void readSensors() {
  float t = dht.readTemperature();
  float h = dht.readHumidity();

  if (!isnan(t)) {
    temperature = t;
  }

  if (!isnan(h)) {
    humidity = h;
  }

  lightValue = analogRead(PIN_LDR);
  soundValue = readSoundPeak();

  Serial.print("Temperature: ");

  if (isnan(temperature)) {
    Serial.print("DHT ERROR");
  } else {
    Serial.print(temperature, 1);
    Serial.print(" C");
  }

  Serial.print(" | Humidity: ");

  if (isnan(humidity)) {
    Serial.print("ERROR");
  } else {
    Serial.print(humidity, 1);
    Serial.print(" %");
  }

  Serial.print(" | LDR: ");
  Serial.print(lightValue);

  Serial.print(" | Sound: ");
  Serial.println(soundValue);
}

// ==================================================
// BUZZER MELODY
// ==================================================

void startTune() {
  if (!playingTune) {
    playingTune = true;
    currentNote = 0;
    nextNoteTime = millis();
  }
}

void stopTune() {
  noTone(PIN_BUZZER);
  playingTune = false;
  currentNote = 0;
}

void playBuzzerTune() {
  if (!playingTune) return;

  unsigned long now = millis();

  if (now < nextNoteTime) return;

  if (currentNote < melodyLength) {
    tone(PIN_BUZZER, melody[currentNote]);

    nextNoteTime =
      now + noteDuration[currentNote];

    currentNote++;
  } else {
    noTone(PIN_BUZZER);

    currentNote = 0;
    nextNoteTime = now + 300;
  }
}

// ==================================================
// FAN CONTROL
// D6 is a control signal, not a motor power supply.
// ==================================================

void setFan(bool on) {
  fanOn = on;

  digitalWrite(PIN_FAN, on ? HIGH : LOW);
  digitalWrite(PIN_FAN2, LOW);
}

// ==================================================
// UPDATE LED, FAN AND BUZZER
// ==================================================

void updateOutputs() {

  // LED: ON in darkness, OFF in bright light.

  if (!lightOn && lightValue > DARK_LIMIT) {
    lightOn = true;
  } else if (
    lightOn &&
    lightValue < DARK_LIMIT - LIGHT_HYST
  ) {
    lightOn = false;
  }

  digitalWrite(PIN_LED, lightOn ? HIGH : LOW);

  // Fan: runs when a registered student is inside
  // and the temperature exceeds the threshold.

  if (insideCount > 0 && !isnan(temperature)) {

    if (!fanOn && temperature > TEMP_LIMIT) {
      setFan(true);
    } else if (
      fanOn &&
      temperature < TEMP_LIMIT - TEMP_HYST
    ) {
      setFan(false);
    }

  } else {
    setFan(false);
  }

  // Sound-triggered buzzer.

  buzzerOn = soundValue > SOUND_LIMIT;

  if (buzzerOn) {
    startTune();
  } else {
    stopTune();
  }
}

// ==================================================
// LCD DISPLAY
// ==================================================

void updateDisplay() {
  if (millis() - lastLCDUpdate < LCD_INTERVAL) {
    return;
  }

  lastLCDUpdate = millis();

  char line1[21];
  char line2[21];
  char line3[21];

  // LINE 1: PROJECT NAME
  lcd.setCursor(0, 0);
  lcd.print("CLASSVITALS        ");

  // LINE 2: TEMPERATURE + DEGREE SYMBOL + FAN
  lcd.setCursor(0, 1);
  lcd.print("                    ");
  lcd.setCursor(0, 1);

  if (isnan(temperature)) {
    lcd.print("Temp:ERR Fan:");
    lcd.print(fanOn ? "ON" : "OFF");
  } else {
    lcd.print("Temp:");
    lcd.print(temperature, 1);

    // Character 223 is the degree symbol on
    // many HD44780-compatible LCDs.
    lcd.write((uint8_t)223);
    lcd.print("C Fan:");
    lcd.print(fanOn ? "ON" : "OFF");
  }

  // LINE 3: LIGHT INTENSITY + LED STATUS
  snprintf(
    line2,
    sizeof(line2),
    "Light:%d LED:%s",
    lightValue,
    lightOn ? "ON" : "OFF"
  );

  lcd.setCursor(0, 2);
  lcd.print("                    ");
  lcd.setCursor(0, 2);
  lcd.print(line2);

  // LINE 4: SOUND READING + BUZZER STATUS
  snprintf(
    line3,
    sizeof(line3),
    "Sound:%d Buzz:%s",
    soundValue,
    buzzerOn ? "ON" : "OFF"
  );

  lcd.setCursor(0, 3);
  lcd.print("                    ");
  lcd.setCursor(0, 3);
  lcd.print(line3);
}

// ==================================================
// FIND REGISTERED STUDENT
// ==================================================

int findStudent(const char *uid) {
  for (byte i = 0; i < STUDENT_COUNT; i++) {
    if (strcmp(students[i].uid, uid) == 0) {
      return i;
    }
  }

  return -1;
}

// ==================================================
// ATTENDANCE PROCESSING
// ==================================================

void handleCard(const char *uid) {

  if (
    strcmp(lastUID, uid) == 0 &&
    millis() - lastCardTime < SAME_CARD_MS
  ) {
    return;
  }

  strncpy(lastUID, uid, sizeof(lastUID) - 1);
  lastUID[sizeof(lastUID) - 1] = '\0';

  lastCardTime = millis();

  int index = findStudent(uid);

  if (index < 0) {
    Serial.print("Unknown card UID: ");
    Serial.println(uid);
    return;
  }

  Student &student = students[index];

  unsigned long nowSeconds = millis() / 1000UL;

  if (!student.inside) {

    student.inside = true;
    student.entrySeconds = nowSeconds;
    insideCount++;

    Serial.print("ENTRY: ");
    Serial.print(student.name);
    Serial.print(" | UID: ");
    Serial.println(uid);

  } else {

    student.inside = false;

    if (insideCount > 0) {
      insideCount--;
    }

    Serial.print("EXIT: ");
    Serial.print(student.name);
    Serial.print(" | Duration: ");
    Serial.print(
      nowSeconds - student.entrySeconds
    );
    Serial.println(" seconds");
  }

  updateOutputs();
}

// ==================================================
// RFID READER
// ==================================================

void checkRFID() {
  if (!rfidOK) return;

  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  char uid[21] = "";
  byte position = 0;

  for (
    byte i = 0;
    i < rfid.uid.size && i < 10;
    i++
  ) {
    char temp[3];

    snprintf(
      temp,
      sizeof(temp),
      "%02X",
      rfid.uid.uidByte[i]
    );

    if (position + 2 < sizeof(uid)) {
      uid[position++] = temp[0];
      uid[position++] = temp[1];
      uid[position] = '\0';
    }
  }

  Serial.print("RFID UID: ");
  Serial.println(uid);

  handleCard(uid);

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}

// ==================================================
// TEST BUTTON
// ==================================================

void checkTestButton() {
  if (
    digitalRead(PIN_BUTTON) == LOW &&
    millis() - lastButtonTime > 300
  ) {
    lastButtonTime = millis();

    Serial.println("Test button pressed");

    handleCard("8C29F506");
  }
}

// ==================================================
// SETUP
// ==================================================

void setup() {
  Serial.begin(9600);

  pinMode(PIN_BUTTON, INPUT_PULLUP);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_FAN, OUTPUT);
  pinMode(PIN_FAN2, OUTPUT);

  digitalWrite(PIN_BUZZER, LOW);
  digitalWrite(PIN_LED, LOW);
  digitalWrite(PIN_FAN, LOW);
  digitalWrite(PIN_FAN2, LOW);

  Wire.begin();

  lcd.init();
  lcd.backlight();
  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("CLASSVITALS");

  lcd.setCursor(0, 1);
  lcd.print("Starting sensors...");

  dht.begin();

  // Initialize RFID.
  pinMode(PIN_SS, OUTPUT);
  digitalWrite(PIN_SS, HIGH);

  SPI.begin();
  rfid.PCD_Init();

  delay(100);

  byte version =
    rfid.PCD_ReadRegister(MFRC522::VersionReg);

  Serial.print("RC522 firmware: 0x");
  Serial.println(version, HEX);

  rfidOK =
    version != 0x00 &&
    version != 0xFF;

  if (rfidOK) {
    Serial.println("RFID communication OK");
  } else {
    Serial.println("RFID communication FAILED");
  }

  // Startup LED and buzzer test.
  digitalWrite(PIN_LED, HIGH);

  tone(PIN_BUZZER, 523);
  delay(200);
  noTone(PIN_BUZZER);

  digitalWrite(PIN_LED, LOW);

  // DHT11 needs time between readings.
  delay(1500);

  readSensors();
  updateOutputs();
  updateDisplay();

  Serial.println("CLASSVITALS ready.");
}

// ==================================================
// MAIN LOOP
// ==================================================

void loop() {
  checkRFID();
  checkTestButton();

  if (
    millis() - lastSensorRead >=
    SENSOR_INTERVAL
  ) {
    lastSensorRead = millis();

    readSensors();
    updateOutputs();
  }

  if (buzzerOn) {
    playBuzzerTune();
  }

  updateDisplay();
}
