#include <OneWire.h>
#include <DallasTemperature.h>

// pin, kde bude senzor zapojený
#define pin 2

// nastavení komunikace senzoru přes pin
OneWire oneWire(pin);     
// převedeme onewire do Dallasu                     
DallasTemperature sensors(&oneWire);           
 
void setup(void)
{
  // nastavení rychlosti sériové komunikace
  Serial.begin(9600);
  // zapnutí senzoru                          
  sensors.begin();                             
}
 
void loop(void)
{
  // požádáme senzor o hodnotu teploty
  sensors.requestTemperatures();               
  Serial.print("Teplota 1 je: ");
  // tento řádek je vysvětlený v článku
  Serial.print(sensors.getTempCByIndex(0));    
  Serial.println(" stupnu Celsia");
  Serial.print("Teplota 2 je: ");
  Serial.print(sensors.getTempCByIndex(1));    
  Serial.println(" stupnu Celsia");
  Serial.println("--------------------------");
  delay(1000);
}
