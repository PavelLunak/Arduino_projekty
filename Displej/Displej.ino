// LCD displej
// navody.arduino-shop.cz
/*
 * LCD RS pin na digital pin 12
 * LCD Enable pin na digital pin 11
 * LCD D4 pin na digital pin 5
 * LCD D5 pin na digital pin 4
 * LCD D6 pin na digital pin 3
 * LCD D7 pin na digital pin 2
 * LCD R/W pin na GND
 * LCD VSS pin na GND
 * LCD VCC pin na 5V
 * LCD V0 pin přes odpor na zem
 *  (kontrast)
 */

// knihovna pro LCD displej
#include <LiquidCrystal.h>

// inicializace pinu, lze vyměnit za jiné volné
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

void setup() {
  // nastavení počtu znaků a řádků LCD, zde 8x2
  lcd.begin(8, 2);
  // vytisknutí hlášky na první řádek
  lcd.setCursor(2, 0);
  lcd.print("TEST");
  // nastavení kurzoru na první znak, druhý řádek
  // veškeré číslování je od nuly, poslední znak je tedy 7, 1
  delay(2000);
}

void loop() {
  // nastaveni kurzoru na osmý znak, druhý řádek
  lcd.setCursor(0, 1);
  // vytisknutí počtu sekund od začátku programu
  lcd.print(millis() / 1000);
}
