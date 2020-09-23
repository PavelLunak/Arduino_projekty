// Teploměr a vlhkoměr DHT11/22

// připojení knihovny DHT
#include "DHT.h"
// nastavení čísla pinu s připojeným DHT senzorem
#define pinDHT 5

// odkomentování správného typu čidla
//#define typDHT11 DHT11     // DHT 11
#define typDHT22 DHT22   // DHT 22 (AM2302)

// inicializace DHT senzoru s nastaveným pinem a typem senzoru
DHT mojeDHT(pinDHT, typDHT22);

void setup() {
  // komunikace přes sériovou linku rychlostí 9600 baud
  Serial.begin(9600); 
  // zapnutí komunikace s teploměrem DHT
  mojeDHT.begin();

  pinMode(2,OUTPUT);
  pinMode(3,OUTPUT);
}

void loop() {
  // pomocí funkcí readTemperature a readHumidity načteme
  // do proměnných tep a vlh informace o teplotě a vlhkosti,
  // čtení trvá cca 250 ms
  float tep = mojeDHT.readTemperature();
  float vlh = mojeDHT.readHumidity();
  // kontrola, jestli jsou načtené hodnoty čísla pomocí funkce isnan
  if (isnan(tep) || isnan(vlh)) {
    // při chybném čtení vypiš hlášku
    Serial.println("Chyba při čtení z DHT senzoru!");
  } else {
    if (tep < 20) {
      digitalWrite(2,HIGH);
      digitalWrite(3,LOW);
    } else {
      digitalWrite(2,LOW);
      digitalWrite(3,HIGH);
    }
    
    // pokud jsou hodnoty v pořádku,
    // vypiš je po sériové lince
    Serial.print("Teplota: "); 
    Serial.print(tep);
    Serial.print(" stupnu Celsia, ");
    Serial.print("vlhkost: "); 
    Serial.print(vlh);
    Serial.println("  %");
  }
  // pauza pro přehlednější výpis
  delay(2000);
}
