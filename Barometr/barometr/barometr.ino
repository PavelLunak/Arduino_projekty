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

#include <LiquidCrystal.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

// nastavení adresy senzoru
#define BMP280_ADRESA (0x76)
// inicializace senzoru BMP z knihovny
Adafruit_BMP280 bmp;
// konstanta s korekcí měření v hPa
int korekce = 32;

String tlakNew = "0";
String teplotaNew = "0";

String tlakOld = "0";
String teplotaOld = "0";

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

void setup() {
  //Serial.begin(9600);
  lcd.begin(8, 2);

  lcd.clear();
  
  lcd.setCursor(0, 0);
  lcd.println("Tlak    ");
  lcd.setCursor(0, 1);
  lcd.println("Teplota ");

  if (!bmp.begin(BMP280_ADRESA)) {
    while (1);
  }

  delay(3000);
}

void loop() {
  float teplota = bmp.readTemperature();
  float tlak = (bmp.readPressure()/100.00) + korekce;

  teplotaNew = String(teplota, 1);
  tlakNew = String(tlak, 0);

  if (isnan(tlak)) {
    clearRow(0);
    lcd.setCursor(2, 0);
    lcd.print("????");
  } else {
    if (tlakNew != tlakOld) {
      clearRow(0);
      lcd.setCursor(0, 0);
      lcd.print(String(tlak, 0));
      lcd.print(" hPa");
      tlakOld = tlakNew;
    }    
  }

  if (isnan(teplota)) {
    clearRow(1);
    lcd.setCursor(2, 1);
    lcd.print("????");
  } else {
    if (teplotaNew != teplotaOld) {
      clearRow(1);
      lcd.setCursor(0, 1);
      lcd.print(String(teplota, 1));
      lcd.print((char)223);
      lcd.print("C");
      teplotaOld = teplotaNew;
    }    
  } 

  delay(2000);
}

void clearRow(int row) {
  lcd.setCursor(0, row);
  lcd.print("        ");
}
