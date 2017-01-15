#include <DHT.h>
#include <DHT_U.h>

#include <EEPROMex.h>
#include <EEPROMVar.h>

#define PINDHT 2
#define PINLOW 7
#define PINMID 8
#define PINHIGH 9
#define DHTTYPE DHT22
#define INTERVAL 10000    // Interval between measurements

float humCurr;
float tempCurrC;
float tempTargetC = 21.1;

int p_restore = 0;
int p_tempTargetC = 4;

int currSpeed;

DHT dht(PINDHT, DHTTYPE);

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
      digitalWrite(PINLOW, LOW);
      digitalWrite(PINMID, LOW);
      digitalWrite(PINHIGH, LOW);
      break;
    case 1:
      digitalWrite(PINLOW, HIGH);
      digitalWrite(PINMID, LOW);
      digitalWrite(PINHIGH, LOW);
      break;
    case 2:
      digitalWrite(PINLOW, LOW);
      digitalWrite(PINMID, HIGH);
      digitalWrite(PINHIGH, LOW);
      break;
    case 3:
      digitalWrite(PINLOW, LOW);
      digitalWrite(PINMID, LOW);
      digitalWrite(PINHIGH, HIGH);
      break;
    default:
      Serial.println("Unknown fan speed given. Working options are 1 (low), 2 (mid) and 3 (high).");
  }
}

void setup() {
  Serial.println("Starting up...");

  EEPROM.write(p_restore, false);

  Serial.begin(9600);
  dht.begin();

  pinMode(PINLOW, OUTPUT);
  pinMode(PINMID, OUTPUT);
  pinMode(PINHIGH, OUTPUT);

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
  delay(INTERVAL);
  
  float dT;

  humCurr = dht.readHumidity();
  tempCurrC = dht.readTemperature();
  dT = tempTargetC - tempCurrC;

//  if (isnan(tempCurrC)) {
//    Serial.println("Failed to read from DHT sensor");
//    return;
//  }

  Serial.print(tempCurrC);
  Serial.print(" degC, target is ");
  Serial.print(tempTargetC);
  Serial.print(" degC, dT = ");
  Serial.print(dT);
  Serial.print(", humidity is ");
  Serial.print(humCurr);
  Serial.println("%");

  if (dT > 3) {
    if (currSpeed != 3) {
      fanSet(3);
    }
  } else if (dT > 1.6) {
    if (currSpeed != 2) {
      fanSet(2);
    }
  } else if (dT > 0.3) {
    if (currSpeed != 1) {
      fanSet(1);
    }
  } else if (dT < -0.3) {
    if (currSpeed != 0) {
      fanSet(0);
    }
  }
}
