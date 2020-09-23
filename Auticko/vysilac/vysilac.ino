#include <SPI.h>
#include "RF24.h"

#define CE 7            // nastavení propojovacích pinů
#define CS 8            // nastavení propojovacích pinů
#define pinPot A0

int val;                //proměnná pro načtení a nastavení úhlu

RF24 nRF(CE, CS);

// nastavení adres pro přijímač a vysílač musí být nastaveny stejně v obou programech!
byte adresaPrijimac[] = "prijimac00";
byte adresaVysilac[] = "vysilac00";

void setup() {
  pinMode(pinPot, INPUT);

  Serial.begin(9600);
  nRF.begin();  
  nRF.setPALevel(RF24_PA_LOW);          // nastavení výkonu nRF modulu (RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH and RF24_PA_MAX)
  nRF.openWritingPipe(adresaVysilac);
}

void loop() {
  val = analogRead(pinPot);             //napětí na potenciometru (0 až 1023)
  val = map(val, 0, 1023, 0, 180);      //převod z 0 až 1023 na 0 až 180

  Serial.print("Posilam uhel natoceni: ");
  Serial.println(val);

  if (!nRF.write( &val, sizeof(val) )) {
    Serial.println("Chyba při odeslání!");
  }
}
