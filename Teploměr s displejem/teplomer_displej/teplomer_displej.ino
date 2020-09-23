/*
   LCD RS pin na digital pin 12
   LCD Enable pin na digital pin 11
   LCD D4 pin na digital pin 5
   LCD D5 pin na digital pin 4
   LCD D6 pin na digital pin 3
   LCD D7 pin na digital pin 2
   LCD R/W pin na GND
   LCD VSS pin na GND
   LCD VCC pin na 5V
   LCD V0 pin přes odpor na zem
    (kontrast)
*/

#include "DHT.h"
#include <LiquidCrystal.h>

#define pinDHT1 6
#define pinDHT2 7
#define typDHT22 DHT22

DHT mojeDHT1(pinDHT1, typDHT22);
DHT mojeDHT2(pinDHT2, typDHT22);

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

void setup() {
  //Serial.begin(9600);
  mojeDHT1.begin();
  mojeDHT2.begin();
  lcd.begin(8, 2);
}

void loop() {

  float tep1 = mojeDHT1.readTemperature();
  float tep2 = mojeDHT2.readTemperature();

  clearRow(0);

  if (isnan(tep1)) {
    clearRow(0);
    lcd.setCursor(2, 0);
    lcd.print("????");
  } else {
    clearRow(0);
    lcd.setCursor(3, 0);
    lcd.print(String(tep1, 1));

    if (tep1 > -10) lcd.print((char)223);
  }

  if (isnan(tep2)) {
    clearRow(1);
    lcd.setCursor(2, 1);
    lcd.print("????");
  } else {
    clearRow(1);
    lcd.setCursor(0, 1);
    lcd.print("IN");
    lcd.print(" ");
    lcd.print(String(tep2, 1));

    if (tep2 > -10) lcd.print((char)223);
  }

  delay(1000);
}

void clearRow(int row) {
  lcd.setCursor(0, row);
  lcd.print("        ");
}
