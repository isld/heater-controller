#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#include <EEPROMex.h>
#include <EEPROMVar.h>

#include <DHT.h>
#include <DHT_U.h>

// button pins 2-4
#define PIN_BTN_DOWN 2
#define PIN_BTN_UP 3
#define PIN_BTN_FAN 4

// sensor pins 
#define PIN_TEMP_WATER 6
#define PIN_TEMP_ROOM 7

// relay control pins
#define PIN_FAN_LOW 8
#define PIN_FAN_MID 9
#define PIN_FAN_HIGH 10
#define PIN_VALVE 11

#define DHT_TYPE DHT22
#define INTERVAL 10000    // Interval between measurements

#define LCD_ADDR 0x3F
#define LCD_COLS 16
#define LCD_ROWS 2

unsigned long timerBtnPress = 0;
unsigned long timerTempRead = 0;

float humCurr;
float tempCurrC;
float heatIndex;
float tempTargetC = 21.1;
float dT;

int p_restore = 0;
int p_tempTargetC = 4;

int buttonPress;

int currFanMode = -1; // -1 = auto, 0/1/2/3
int currSpeed = 0;

DHT dht(PIN_TEMP_ROOM, DHT_TYPE);
LiquidCrystal_I2C lcd(LCD_ADDR, LCD_COLS, LCD_ROWS);


void setup() {
//  EEPROM.write(p_restore, false);

  Serial.begin(9600);
  dht.begin();

  Serial.println("Starting up...");

  pinMode(PIN_FAN_LOW, OUTPUT);
  pinMode(PIN_FAN_MID, OUTPUT);
  pinMode(PIN_FAN_HIGH, OUTPUT);
  pinMode(PIN_VALVE, OUTPUT);

  pinMode(PIN_BTN_DOWN, INPUT);
  pinMode(PIN_BTN_UP, INPUT);
  pinMode(PIN_BTN_FAN, INPUT);

  bool restore = EEPROM.read(p_restore);

  if (restore) {
    Serial.println("Restoration flag is high, restoring target temperature");

    tempTargetC = EEPROM.readFloat(p_tempTargetC);

    Serial.write("Target temperature restored to "); Serial.println(tempTargetC);
  } else {
    Serial.println("Writing default target temperature...");

    EEPROM.update(p_restore, true);
    EEPROM.updateFloat(p_tempTargetC, tempTargetC);
  }

  lcd.init();
  lcd.backlight();
  lcd.print("Hello!");

  dht.begin();
}
 
void loop() {
  if (millis() - timerTempRead > INTERVAL) {
    timerTempRead = millis();
    
    readTemp();
  
    if (isnan(tempCurrC)) {
      Serial.println("Failed to read from DHT sensor");
      return;
    }
  
    Serial.print(tempCurrC);
    Serial.print(" degC, heat index is ");
    Serial.print(heatIndex);
    Serial.print(" degC, target is ");
    Serial.print(tempTargetC);
    Serial.print(" degC, dT = ");
    Serial.print(dT);
    Serial.print(", humidity is ");
    Serial.print(humCurr);
    Serial.println("%");

    updateSpeed();
    updateLCD();
  }

  readButtons();
}

void readButtons() {
  if (millis() - timerBtnPress < 200) {
    return 1;
  }
  
  buttonPress = digitalRead(PIN_BTN_UP);

  if (buttonPress == HIGH) {
    timerBtnPress = millis();
    if (tempTargetC < 32) {
      tempTargetC = tempTargetC + 0.1;
      EEPROM.updateFloat(p_tempTargetC, tempTargetC);

      updateState();
    }
  }

  buttonPress = digitalRead(PIN_BTN_DOWN);

  if (buttonPress == HIGH) {
    timerBtnPress = millis();
    if (tempTargetC > 13) {
      tempTargetC = tempTargetC - 0.1;
      EEPROM.updateFloat(p_tempTargetC, tempTargetC);

      updateState();
    }
  }

  buttonPress = digitalRead(PIN_BTN_FAN);

  if (buttonPress == HIGH) {
    timerBtnPress = millis();
    currFanMode = currFanMode + 1;

    if (currFanMode > 3) {
      currFanMode = -1;
    }

    updateState();
  }
}

void updateState() {
  updateLCD();
  readTemp();
  updateSpeed();
  updateLCD();
}

void readTemp() {
  humCurr = dht.readHumidity();
  tempCurrC = dht.readTemperature();
  dT = tempTargetC - tempCurrC;
  heatIndex = dht.computeHeatIndex(tempCurrC, humCurr, false);
}

void updateSpeed() {
  if (dT < -0.3 || currFanMode == 0) {
    if (currSpeed != 0) {
      fanSet(0);
    }
    return;
  }

  if (dT > 0.3) {
    if (currFanMode != -1) {
      if (currSpeed != currFanMode) {
        fanSet(currFanMode);
      }
      return;
    }

    if (dT <= 0.6) {
      if (currSpeed != 1) {
        fanSet(1);
      }
      return;
    }

    if (dT <= 3) {
      if (currSpeed != 2) {
        fanSet(2);
      }
      return;
    }

    if (currSpeed != 3) {
      fanSet(3);
    }
  }
}

void updateLCD() {
  lcd.setCursor(0, 0);
  lcd.print("T: ");
  lcd.print(tempCurrC);
  lcd.print(" / ");
  lcd.print(tempTargetC);
  lcd.setCursor(0, 1);
  lcd.print("H: ");
  lcd.print(humCurr);
  lcd.print("% F: ");
  lcd.print(currSpeed);
  lcd.print("/");
  lcd.print(currFanMode);
}

void fanSet(int speed) {
  Serial.print("Current temperature is ");
  Serial.print(tempCurrC);
  Serial.print(" degC, target is ");
  Serial.print(tempTargetC);
  Serial.print(". Switching fan to speed ");
  Serial.print(speed);
  Serial.println(" where 1=low, 2=mid and 3=high");

  currSpeed = speed;

  switch (speed) {
    case 0:
      digitalWrite(PIN_FAN_LOW, LOW);
      digitalWrite(PIN_FAN_MID, LOW);
      digitalWrite(PIN_FAN_HIGH, LOW);
      break;
    case 1:
      digitalWrite(PIN_FAN_LOW, HIGH);
      digitalWrite(PIN_FAN_MID, LOW);
      digitalWrite(PIN_FAN_HIGH, LOW);
      break;
    case 2:
      digitalWrite(PIN_FAN_LOW, LOW);
      digitalWrite(PIN_FAN_MID, HIGH);
      digitalWrite(PIN_FAN_HIGH, LOW);
      break;
    case 3:
      digitalWrite(PIN_FAN_LOW, LOW);
      digitalWrite(PIN_FAN_MID, LOW);
      digitalWrite(PIN_FAN_HIGH, HIGH);
      break;
    default:
      Serial.println("Unknown fan speed given.");
  }
}

