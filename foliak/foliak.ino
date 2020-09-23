#include <LiquidCrystal.h>
#include "DHT.h"

#define pinVent 8
#define pinDHT1 9

DHT tempSensor(pinDHT1, DHT22);
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

boolean ventIsRunning = false;

int temperature = 0;
int humidity = 0;
int maxTemperature = 28;
int maxHumidity = 70;

String temperatureToShow = "";
String humidityToShow = "";

String lastTemperatureToShow = "";
String lastHumidityToShow = "";

int counter = 0;

void setup() {
  //Serial.begin(9600);
  
  pinMode(pinVent, OUTPUT);
  digitalWrite(pinVent, LOW);

  lcd.begin(8, 2);
  tempSensor.begin();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print(" Foliak");
  delay(1000);
  lcd.clear();
}

void loop() {
  checkTemperature();
  checkHumidity();
  checkVent();
  delay(1000);
}

void checkTemperature() {
  temperature = tempSensor.readTemperature();

  if (isnan(temperature)) {
    temperature = 0;
    temperatureToShow = "--";
  } else {
    temperatureToShow = String(temperature);
  }

  compareValues(1, temperatureToShow, lastTemperatureToShow);
}

void checkHumidity() {
  humidity = tempSensor.readHumidity();

  if (isnan(humidity)) {
    humidity = 0;
    humidityToShow = "--";
  } else {
    humidityToShow = String(humidity);       
  }

  compareValues(2, humidityToShow, lastHumidityToShow);
}

void checkVent() {
  if (temperature > maxTemperature) {
    maxTemperature = 26;
    startVent(true);
  } else {
    if (humidity > maxHumidity) {
      maxHumidity = 67;
      startVent(true);
    } else {
      startVent(false);
      maxTemperature = 28;
      maxHumidity = 70;
    }
  }
}

void startVent(boolean start) {
  if (start) {
    if (ventIsRunning) return;
    digitalWrite(pinVent, HIGH);
    ventIsRunning = true;
    counter ++;
    updateCounter();
  }
  else {
    if (!ventIsRunning) return;
    digitalWrite(pinVent, LOW);
    ventIsRunning = false;
  }
}

void updateTemp() {  
  lcd.setCursor(0, 0);
  lcd.print("    ");
  lcd.setCursor(0, 0);
  lcd.print(lastTemperatureToShow);
  lcd.print((char)223);
}

void updateHumidity() { 
  lcd.setCursor(5, 0);
  lcd.print("   ");
  lcd.setCursor(5, 0);
  lcd.print(lastHumidityToShow);
  lcd.print("%");
}

void updateCounter() { 
  lcd.setCursor(0, 1);
  lcd.print("        ");
  lcd.setCursor(0, 1);
  lcd.print(String(counter));
}

/*
Konstanty pro rozlišení typů hodnot
 teplota = 1
 vlhkost = 2
*/
void compareValues(byte valueType, String value1, String value2) {
  if (value1 != value2) {
    switch (valueType) {
      case 1:
        lastTemperatureToShow = temperatureToShow;
        updateTemp();
        break;
      case 2:
        lastHumidityToShow = humidityToShow;
        updateHumidity();
        break;
    }
  }
}
