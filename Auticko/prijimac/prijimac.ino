#include <SPI.h>
#include "RF24.h"
#include <Servo.h>

#define CE 7        // nastavení propojovacích pinů
#define CS 8        // nastavení propojovacích pinů

Servo myservo;
int pos = 0;

RF24 nRF(CE, CS);

// nastavení adres pro přijímač a vysílač, musí být nastaveny stejně v obou programech!
byte adresaPrijimac[] = "prijimac00";
byte adresaVysilac[] = "vysilac00";

void setup() {
  Serial.begin(9600);
  nRF.begin();
  nRF.setPALevel(RF24_PA_LOW);          // nastavení výkonu nRF modulu (RF24_PA_MIN, RF24_PA_LOW, RF24_PA_HIGH and RF24_PA_MAX)
  nRF.openReadingPipe(1, adresaVysilac);
  nRF.startListening();

  myservo.attach(9);   //motor je připojen na pin 9  
}

void loop() {
  int prijem;

  if (nRF.available()) {
    while (nRF.available()) {
      nRF.read(&prijem, sizeof(prijem));
    }

    Serial.print("Prijaty uhel natoceni: ");
    Serial.print(prijem);
    myservo.write(prijem);
  }
}
