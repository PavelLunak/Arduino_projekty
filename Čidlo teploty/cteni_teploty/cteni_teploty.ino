#include <OneWire.h>
#include <DallasTemperature.h>

OneWire  ds(10);  // onewire sběrnice na pinu 10 jménem ds

float teplota;

DallasTemperature sensors(&ds);  // čidlo teploty na sběrnici ds

DeviceAddress cidlo = { 0x28, 0xFF, 0x62, 0x1B, 0x90, 0x15, 0x01, 0x13 }; // adresa čidla jménem cidlo

void setup() 
{
  Serial.begin(9600);  // spuštění sériového portu

  sensors.begin(); // spuštění čidel

  sensors.setResolution(cidlo, 10);  // nastavení rozlišení čidla na 10 bitů

}

void loop() 
{
  sensors.requestTemperatures();

  teplota = sensors.getTempC(cidlo); // načtění teploty ve stupních celsia

  Serial.println(teplota);  // vypiš teplotu

  delay(2000);  // čekej

  
 
}
