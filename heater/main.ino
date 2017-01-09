#include <EEPROMex.h>
#include <EEPROMVar.h>

int pinTemp = 0;
int pinLow = 7;
int pinMid = 8;
int pinHigh = 9;

float tempCurrC;
float tempTargetC = 16.8;

int p_restore = 0;
int p_tempTargetC = 4;

int currSpeed;

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
      digitalWrite(pinLow, LOW);
      digitalWrite(pinMid, LOW);
      digitalWrite(pinHigh, LOW);
      break;
    case 1:
      digitalWrite(pinLow, HIGH);
      digitalWrite(pinMid, LOW);
      digitalWrite(pinHigh, LOW);
      break;
    case 2:
      digitalWrite(pinLow, LOW);
      digitalWrite(pinMid, HIGH);
      digitalWrite(pinHigh, LOW);
      break;
    case 3:
      digitalWrite(pinLow, LOW);
      digitalWrite(pinMid, LOW);
      digitalWrite(pinHigh, HIGH);
      break;
    default:
      Serial.println("Unknown fan speed given. Working options are 1 (low), 2 (mid) and 3 (high).");
  }
}

void setup() {
  Serial.println("Starting up...");

  EEPROM.write(p_restore, false);

  Serial.begin(9600);
  analogReference(EXTERNAL);

  pinMode(pinLow, OUTPUT);
  pinMode(pinMid, OUTPUT);
  pinMode(pinHigh, OUTPUT);

  bool restore = EEPROM.read(p_restore);

  if (restore) {
    Serial.println("Restoration flag is high, restoring target temperature");

    tempTargetC = EEPROM.readFloat(p_tempTargetC);

    Serial.write("Target temperature restored to "); Serial.println(tempTargetC);
  } else {
    Serial.println("Restoration flag is low, setting it high and writing default target temperature...");

    EEPROM.update(p_restore, true);
    EEPROM.updateFloat(p_tempTargetC, tempTargetC);
  }
}

void loop() {
  int readTemp = analogRead(pinTemp);
  float voltage = readTemp * 3.3 / 1024.0;
  float dT;

  tempCurrC = (voltage - 0.5) * 100;
  dT = tempTargetC - tempCurrC;

  Serial.print(tempCurrC);
  Serial.print(" degC, target is ");
  Serial.print(tempTargetC);
  Serial.print(" degC, dT = ");
  Serial.println(dT);

  if (dT > 3) {
    if (currSpeed != 3) {
      fanSet(3);
    }
  } else if (dT > 2) {
    if (currSpeed != 2) {
      fanSet(2);
    }
  } else if (dT > 1) {
    if (currSpeed != 1) {
      fanSet(1);
    }
  } else if (dT < -1) {
    if (currSpeed != 0) {
      fanSet(0);
    }
  }

  delay(10000);
}
