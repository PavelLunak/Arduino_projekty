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

#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal.h>

#define pinKompresor 6
#define pinTeplotaLednice 9
#define pinTeplotaMrazaku 8

const float maxTeplota = 7.5;
const float minTeplota = 6.5;
const float maxTeplotaMrazaku = -18.0;

const long prodlevaPoPripojeni = 60000;       //1 minuta
const long maxDobaBehu = 1800000;             //30 minut
const long pauza = 600000;                    //10 minut
const long toleranceChybyCidla = 300000;      //5 minut
const long nouzeMaxDobaBehu = 1200000;        //20 minut
const long nouzePauza = 2700000;              //45 minut

OneWire oneWireLednice(pinTeplotaLednice);      //Nastavení komunikace senzoru přes pin
OneWire oneWireMrazak(pinTeplotaMrazaku);       //Nastavení komunikace senzoru přes pin
DallasTemperature lednice(&oneWireLednice);     //Převedeme onewire do Dallasu
DallasTemperature mrazak(&oneWireMrazak);       //Převedeme onewire do Dallasu
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);

float teplotaLednice = 0;
float teplotaMrazaku = 0;

String teplotaLedniceString = "";
String teplotaMrazakuString = "";

boolean jeSnizovaniTeploty = false;
boolean jePauza = false;
boolean jeChybaMereni = false;
boolean jeVadneCidlo = false;
boolean poZapnuti = true;
boolean prvniStartKompresoru = true;

unsigned long casPripojeni = 0;
unsigned long posledniStartKompresoru = 0;
unsigned long posledniStopKompresoru = 0;
unsigned long casChybyCteniTeploty = 0;

//long pocitadloPauza = 0;

void setup() {

  pinMode(pinKompresor, OUTPUT);
  digitalWrite(pinKompresor, LOW);
  
  //Serial.begin(9600);
  
  lednice.begin();
  mrazak.begin();
  
  lcd.begin(8, 2);
}

void loop() {
  cekejPoPripojeniDoSite();

  zkontrolujPauzu();
  zkontrolujDelkuBehuKompresoru();
  
  zjistiTeplotuLednice();
  zjistiTeplotuMrazaku();
  
  zkontrolujNactenouTeplotu();
  zapniVypniKompresor();
  
  zobrazTeplotuLednice();
  zobrazTeplotuMrazaku();

  zobrazPauzu();
  zobrazMinuty();
  
  delay(10000);
}

void cekejPoPripojeniDoSite() {
  if (poZapnuti == false) {
    return;
  }

  zjistiTeplotuLednice();
  zjistiTeplotuMrazaku();
  zobrazTeplotuLednice();
  zobrazTeplotuMrazaku();
  zobrazStart();

  long counter = 0;

  while (counter < prodlevaPoPripojeni) {
    counter = counter + 1000;
    delay(1000);
  }

  poZapnuti = false;
  zobrazStart();
}

void zjistiTeplotuLednice() {
  lednice.requestTemperatures();
  teplotaLednice = lednice.getTempCByIndex(0);

  if (isnan(teplotaLednice) || (teplotaLednice < -100)) {
    teplotaLednice = 0.0;
    teplotaLedniceString = "---";
    jeChybaMereni = true;
  } else {
    teplotaLedniceString = String(teplotaLednice, 1);
    jeChybaMereni = false;
    jeVadneCidlo = false;
    casChybyCteniTeploty = 0;
  }
}

void zjistiTeplotuMrazaku() {
  mrazak.requestTemperatures();
  teplotaMrazaku = mrazak.getTempCByIndex(0);

  if (isnan(teplotaMrazaku) || (teplotaMrazaku < -100)) {
    teplotaMrazaku = 0.0;
    teplotaMrazakuString = "---";
  } else {
    teplotaMrazakuString = String(teplotaMrazaku, 1);
  }
}

void zkontrolujNactenouTeplotu() {
  if (jeChybaMereni == false) {
    return;
  }

  // Právě došlo k chybě čtení teploty
  if (casChybyCteniTeploty == 0) {
    casChybyCteniTeploty = millis();
    return;
  }
  // Uplynul čas pro toleranci chyby měření
  else if ((millis() - casChybyCteniTeploty) > toleranceChybyCidla) {
    jeVadneCidlo = true;
  }
}

void zapniVypniKompresor() {
  if ((teplotaLednice <= minTeplota) && (jePotrebaChladitMrazak() == false)) {
    vypniKompresor();
    //jePauza = false; // Zrušení pauzy, kterou nastavila metoda vypniKompresor()
    jeSnizovaniTeploty = false;
  } else if (jeSnizovaniTeploty) {
    if (jePauza == false) {
      zapniKompresor();
    }
  } else if ((teplotaLednice >= maxTeplota) || (jePotrebaChladitMrazak() == true)) {    
    jeSnizovaniTeploty = true;
    
    if (jePauza == false) {
      zapniKompresor();
    }
  }
}

boolean jePotrebaChladitMrazak() {
  if ((teplotaMrazaku > maxTeplotaMrazaku) && (teplotaMrazaku != 0.0)) {
    return true;
  } else {
    return false;
  }
}

void zkontrolujPauzu() {
  if (jePauza == false) {
    return;
  }

  if ((millis() - posledniStopKompresoru) > zjistiDelkuPauzy()) {
    jePauza = false;
  }
}

boolean zkontrolujDelkuBehuKompresoru() {
  if (jePauza == true) {
    return;
  }
  
  if ((millis() - posledniStartKompresoru) > zjistiDelkuBehu()) {
    vypniKompresor();
  }
}

long zjistiDelkuPauzy() {
  if (jeVadneCidlo) {
    return nouzePauza;
  } else {
    return pauza;
  }
}

long zjistiDelkuBehu() {
  if (jeVadneCidlo) {
    return nouzeMaxDobaBehu;
  } else {
    return maxDobaBehu;
  }
}

void zapniKompresor() {
  prvniStartKompresoru = false;
  
  if (digitalRead(pinKompresor) == HIGH) {
    return;
  }

  digitalWrite(pinKompresor, HIGH);
  posledniStartKompresoru = millis();
  posledniStopKompresoru = 0;
}

void vypniKompresor() {
  if (digitalRead(pinKompresor) == LOW) {
    return;
  }
  
  digitalWrite(pinKompresor, LOW);
  posledniStopKompresoru = millis();
  posledniStartKompresoru = 0;

  // Po každém vypnutí kompresoru je nutné zajistit aby se znovu hned nerozběhl
  jePauza = true;
}

void zobrazTeplotuLednice() {  
  lcd.setCursor(0, 0);
  lcd.print("     ");
  lcd.setCursor(0, 0);
  lcd.print(teplotaLedniceString);
  lcd.print((char)223);
}

void zobrazTeplotuMrazaku() {  
  lcd.setCursor(0, 1);
  lcd.print("      ");
  lcd.setCursor(0, 1);
  lcd.print(teplotaMrazakuString);
  lcd.print((char)223);
}

void zobrazPauzu() {  
  lcd.setCursor(5, 0);

  if (jePauza) {
    lcd.print("P");
  } else {
    lcd.print(" ");
  }
}

void zobrazStart() {
  lcd.setCursor(5, 0);

  if (poZapnuti) {
    lcd.print("S");
  } else {
    lcd.print(" ");
  }
}

void zobrazMinuty() {
  long hodnota = posledniStartKompresoru;
  if (hodnota == 0) {
    hodnota = posledniStopKompresoru;
  }

  if (hodnota == 0 || ((jeSnizovaniTeploty == false) && (jePauza == false))) {
    vymazMinuty();
    return;
  }
  
  int cas = ((millis() - hodnota) / 1000 / 60);

  lcd.setCursor(6, 0);

  if (cas <= 0) {
    lcd.print("  ");
    return;
  }
  
  if (cas < 10) {
    lcd.print(" ");
    lcd.print(cas);
  } else {
    lcd.print(cas);
  }
}

void vymazMinuty() {
  lcd.setCursor(6, 0);
  lcd.print("  ");
}
