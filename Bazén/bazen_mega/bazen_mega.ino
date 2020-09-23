#include <OneWire.h>
#include <DallasTemperature.h>
#include <LiquidCrystal.h>
#include "DHT.h"

// 4 Displej
// 5 Displej
// 6 Displej
// 7 Displej
#define pinButtonStartStop 2        //Tlačítko START/STOP (interrupt pin)
#define pinButtonMode 3             //Změna režimu zobrazení dat (interrupt pin)
#define pinTemperaturePool 24       //Teplota vody bazenu                    
#define pinDysplayBackLight 25      //Napájení podsvícení displeje
#define pinPumpInWater 26           //Hladinový spínač v prostoru čerpadla
// 11 Displej
// 12 Displej
#define pinPumpVent 44              //Ventilátor čerpadla
#define pinTemperatureInOut 8//28      //Čidla teplot přítoku a odtoku
#define pinRelayOn 29
#define pinDHT1 42                  //Teplotní snímač u čerpadla
#define pinPumpWithoutWater 31      //Hladinový spínač - přítomnost vody v bazénu
#define pinModeOnlyFilter 32        //Přepínač pro vyřazení hlídání teploty vody a intenzity světla   

#define pinLed 33                   //LED - zařízení v činnosti    
#define pinBeep 34                  //Pípáček    

#define delayForPoolTemperatureMeasurement 1000   //1 sekunda - interval měření teploty bazénu
#define delayForInOutTemperatureMeasurement 1000  //1 sekunda - interval měření teploty vstupu a výstupu
#define delayAfterPumpWithoutWaterMillis 10000    //10 sekund - prodleva po zjištění nedostatku vody v sání
#define delayModePressMillis 1000                 //1 sekunda - Protizákmit tlačítka mode. Po stisku nebude 1s reagovat
#define delayStartStopPressMillis 1000            //1 sekunda - Protizákmit tlačítka START/STOP. Po stisku nebude 3s reagovat
#define delayDisplayBackLightMillis 600000        //10 minut - doba, po které zhasne podsvícení displeje
#define delayForOnlyFilterSwitchMillis 1000       //1 sekunda - doba, po které zkontroluje stav vypínače po jeho přestavení

const long delayDryingMillis = 1200000;           //20 minut - pro sušení čerpadla po jeho zaplavení
const int delayBeepMillis = 1000;                  //Pípnutí při stisku tlačítka;

const byte charTempWaterAlertIndex = 2;
const byte charNoWaterAlertIndex = 6;
const byte charPumpInWaterAlertIndex = 7;
const byte charPumpWetAlertIndex = 10;
const byte charVentAlertIndex = 11;

const byte charTempWaterAlert[8] =    {B00001, B00001, B00001, B01000, B11101, B01000, B01000, B01100}; // ("V" a pod nim v rohu trojúhelník)
const byte charNoWaterAlert[8] =      {B11100, B10100, B11100, B10000, B10000, B00101, B00010, B00101}; // ("P" a pod nim křížek)
const byte charPumpInWaterAlert[8] =  {B01110, B01010, B01110, B01000, B01000, B00000, B01101, B10010}; // ("P" a pod nim vlnovka)
const byte charPumpWetAlert[8] =      {B00100, B00100, B01010, B01010, B10001, B10001, B10001, B01110}; // (kapka)
const byte charVentAlert[8] =         {B00000, B00000, B11011, B11011, B00100, B11011, B11011, B00000}; // (vrtulka)

OneWire oneWireInOut(pinTemperatureInOut);        //Nastavení komunikace senzoru přes pin
OneWire oneWirePool(pinTemperaturePool);          //Nastavení komunikace senzoru přes pin
DallasTemperature sensorsInOut(&oneWireInOut);    //Převedeme onewire do Dallasu
DallasTemperature sensorPool(&oneWirePool);       //Převedeme onewire do Dallasu
DHT pumpTempSensor(pinDHT1, DHT11);
LiquidCrystal lcd(12, 11, 7, 6, 5, 4);

/*
   LCD RS pin na digital pin 12
   LCD Enable pin na digital pin 11
   LCD D4 pin na digital pin 7
   LCD D5 pin na digital pin 6
   LCD D6 pin na digital pin 5
   LCD D7 pin na digital pin 4
   LCD R/W pin na GND
   LCD VSS pin na GND
   LCD VCC pin na 5V
   LCD V0 pin přes odpor na zem
    (kontrast)
*/

//Hodnoty z čidel
float tempWaterIn = 0;
float tempWaterOut = 0;
float tempWaterPool = 0;
float pumpTemperature = 0;
float pumpHumidity = 0;

//Hodnoty z čidel k zobrazení (po zaokrouhlení)
String tempWaterInToShow = "";
String tempWaterOutToShow = "";
String tempWaterPoolToShow = "";
String pumpTemperatureToShow = "";
String pumpHumidityToShow = "";

String lastTempWaterInToShow = "";
String lastTempWaterOutToShow = "";
String lastTempWaterPoolToShow = "";
String lastPumpTemperatureToShow = "";
String lastPumpHumidityToShow = "";

unsigned long pumpWithoutWaterTimeActivation = 0;   //Čas ztráty vody v čerpadle
unsigned long pumpInWaterTimeActivation = 0;        //Čas zaplavení prostoru čerpadla vodou
unsigned long waterLevelDropTime = 0;               //Čas, kdy došlo k poklesu hladiny po předchozím zaplavení prostoru čerpadla
unsigned long buttonModeTimeActivation = 0;         //Čas posledního stisku tlačítka MODE
unsigned long buttonStartStopTimeActivation = 0;    //Čas posledního stisku tlačítka START/STOP
unsigned long displayBackLightOnTimeActivation = 0; //Čas rozsvícení podsvícení displeje
unsigned long lastOnlyFilterSwitchMoved = 0;        //Čas poslední změny polohy přepínače "pouze filtrování"
unsigned long lastPoolTemperatureMeasurement = 0;   //Čas posledního měření teploty bazénu
unsigned long lastInOutTemperatureMeasurement = 0;  //Čas posledního měření teploty vstupu a výstupu

boolean isRequestStopAfterTurningOn = false;        //Po zapnutí napájení požadavek na NEspuštění čerpadla
boolean isBackLight = false;                        //Příznak, že svítí (nesvítí) podsvícení displeje
boolean isManualOff = false;                        //Příznak, že bylo čerpadlo vypnuto ručně
boolean isOnlyFilter = false;                       //Příznak, že je ručně nastaveno pouze filtrování (vyřazení detekce světla a teplot vody)
int lastIsOnlyFilter = LOW;

int waitForStopCounter = 0;

boolean isFirstLoop = true;                 //Příznak, že jde o první cyklus po zapnutí napájení
boolean deviceStartet = false;              //Příznak, že bylo zařízení uvedeno tlačítkem do činnosti
boolean pumpIsRunning = false;              //Příznak, že čerpadlo běží
boolean pumpVentIsRunning = false;          //Příznak, že běží ventilátor čerpadla

boolean hasMaxTemp = false;                 //Dosaženo maximální teploty vody v bazénu
boolean isPumpWithoutWater = false;         //V čerpadle není voda
boolean isPumpInWater = false;              //Prostor čerpadla je zaplavený
boolean isPumpWet = false;                  //Čerpadlo je mokré (po zatopení a poklesu hladiny)
boolean isPumpWarm = false;                 //Čerpadlo je teplé (nutno začít chladit)
boolean isPumpHot = false;                  //Čerpadlo je přehřáté
boolean isTooHighHumidity = false;          //Vysoká vlhkost v prostoru čerpadla

//mode 0  hlavní zobrazení - pouze teplota v bazénu
//mode 1  zobrazení hodnot z čidel
byte modeSelect = 0;

boolean requestStartStop = false;
boolean requestMode = false;


void setup(void) {

  //Tlačítka ovládání start/stop
  pinMode(pinButtonStartStop, INPUT);

  //LED
  pinMode(pinLed, OUTPUT);
  digitalWrite(pinLed, LOW);

  //Beep
  pinMode(pinBeep, OUTPUT);
  digitalWrite(pinBeep, LOW);

  //Relé pro START
  pinMode(pinRelayOn, OUTPUT);
  digitalWrite(pinRelayOn, LOW);

  //Ventilátor čerpadla
  pinMode(pinPumpVent, OUTPUT);
  digitalWrite(pinPumpVent, HIGH);

  //Hladinový spínač - voda u čerpadla
  pinMode(pinPumpInWater, INPUT);

  //Hladinový spínač - voda v bazénu
  pinMode(pinPumpWithoutWater, INPUT);

  //Tlačítko volby režimu zobrazení dat na displeji
  pinMode(pinButtonMode, INPUT);

  //Výstup pro napájení podsvícení displeje
  pinMode(pinDysplayBackLight, OUTPUT);
  digitalWrite(pinDysplayBackLight, LOW);

  //Přepínač pro zapnutí pouze filtrování
  pinMode(pinModeOnlyFilter, INPUT);

  backlight(true);

  /*
    LOW — přerušení nastane vždy, když je pin v logické nule.
    CHANGE — přerušení nastane při změně logické hodnoty na daném pinu.
    RISING — přerušení s příchodem vzestupné hrany.
    FALLING — přerušení s příchodem sestupné hrany.

    Piny pro přerušení u Arduina UNO - 2, 3
    Piny pro přerušení u Arduina MEGA - 2, 3, 18, 19, 20, 21
  */
  attachInterrupt(digitalPinToInterrupt(pinButtonStartStop), onButtonStartStopPressed, RISING);
  attachInterrupt(digitalPinToInterrupt(pinButtonMode), onButtonModePressed, RISING);

  Serial.begin(9600);       //nastavení rychlosti sériové komunikace
  sensorsInOut.begin();     //zapnutí senzorů teploty přítoku a odtoku
  sensorPool.begin();       //zapnutí senzorů teploty bazénu
  pumpTempSensor.begin();   //zapnutí teplotního čidla čerpadla

  //Inicializace displeje
  lcd.begin(20, 4);
  lcd.clear();
  createCustomChars();
  lcd.setCursor(0, 0);
  lcd.print("*******BAZEN*******");
  lcd.setCursor(0, 1);
  lcd.print(" ================== ");
  lcd.setCursor(0, 2);
  lcd.print("                    ");
  lcd.setCursor(0, 3);
  lcd.print("Tyfole necum na mee!");
  
  delay(3000);
  lcd.clear();
}

void loop(void) {

  // Odpočet před startem čerpadla po zapnutí zařízení
  if (!deviceStartet) {    
    if (isFirstLoop) {
      backlight(true);
      waitForStop();
      lcd.clear();
    }
  }

  checkBackLightTime();

  checkTemperatureInOut();  //Zjištění teplot In a Out
  checkTemperaturePool();   //Zjištění teploty vody v bazénu
  checkPumpWithoutWater();  //Test, jestli je v čerpadle voda
  checkPumpInWater();       //Test, jestli není prostor čerpadla zaplaven
  checkPumpTemp();          //Zjištění teploty v okolí čerpadla
  checkHumidity();          //Zjištění vlhkosti v prostoru čerpadla
  checkVentRequire();       //Zjištění potřeby zapnout ventilaci
  checkOnlyFilter();        //Zapnuta pouze filtrace bez hlídání světla a teplot vody

  if (checkInputs()) startSsrRelay(false);
  else startSsrRelay(true);

  // TLAČÍTKO START/STOP
  if (requestStartStop) {
    requestStartStop = false;

    if (!isBackLight) {
      backlight(true);
      return;
    }

    backlight(true);

    if (deviceStartet) {
      isManualOff = true;
      startSsrRelay(false);
    } else {
      isManualOff = false;
      if (!checkInputs) {
        startSsrRelay(true);
      }
    }
  }

  // TLAČÍTKO MODE
  if (requestMode) {
    requestMode = false;

    if (!isBackLight) {
      backlight(true);
      return;
    }

    backlight(true);

    if (modeSelect == 0)  modeSelect = 1;
    else modeSelect = 0;

    lcd.clear();

    if (modeSelect == 0) {
      updatePoolTemp();
    } else {
      updatePoolTemp();
      updateInTemp();
      updateOutTemp();
      updatePumpTemp();
      updatePumpHumidity();
    }
  }

  updateStatusBar();
  //updateDataOnDisplay();

  isFirstLoop = false;
}

void createCustomChars() {
  lcd.createChar(charTempWaterAlertIndex, charTempWaterAlert);
  lcd.createChar(charNoWaterAlertIndex, charNoWaterAlert);
  lcd.createChar(charPumpInWaterAlertIndex, charPumpInWaterAlert);
  lcd.createChar(charPumpWetAlertIndex, charPumpWetAlert);
  lcd.createChar(charVentAlertIndex, charVentAlert);
}

boolean checkInputs() {
  return hasMaxTemp || isPumpInWater || isPumpWithoutWater || isPumpWet || isPumpHot;
}

void checkBackLightTime() {
  if (!isBackLight) return;
  if ((millis() - displayBackLightOnTimeActivation) > delayDisplayBackLightMillis) {
    backlight(false);
    displayBackLightOnTimeActivation = 0;
  }
}

void onButtonStartStopPressed() {
  //Protizákmitová ochrana tlačítka
  if ((millis() - buttonStartStopTimeActivation) > delayStartStopPressMillis) {
    buttonStartStopTimeActivation = millis();
  } else {
    return;
  }

  beep();

  //Důležité jen při zapnutí napájení zařízení.
  //Slouží k ukončení odpočtu do zapnutí čerpadla a pro NEzapnutí zařízení po zapnutí napájení.
  isRequestStopAfterTurningOn = true;

  if (!isFirstLoop) requestStartStop = true;
  return;
}

void onButtonModePressed() {
  if ((millis() - buttonModeTimeActivation) > delayModePressMillis) {
    buttonModeTimeActivation = millis();
  } else {
    return;
  }

  beep();

  //Důležité jen při zapnutí napájení zařízení.
  //Slouží k ukončení odpočtu do zapnutí čerpadla a pro NEzapnutí zařízení po zapnutí napájení.
  isRequestStopAfterTurningOn = true;

  if (!isFirstLoop) requestMode = true;
  return;
}

void checkPumpTemp() {
  pumpTemperature = pumpTempSensor.readTemperature();

  if (isnan(pumpTemperature)) {
    pumpTemperature = 0.0;
    pumpTemperatureToShow = "---";
  } else {
    pumpTemperatureToShow = String(pumpTemperature, 0);
  }

  compareValues(4, pumpTemperatureToShow, lastPumpTemperatureToShow);

  if (pumpTemperature > 0 || isPumpHot) {
    int maxTemp2 = 45;
    
    if (isPumpHot) maxTemp2 = 40;
    
    if (pumpTemperature > maxTemp2) isPumpHot = true;
    else isPumpHot = false;
  }
}

void checkHumidity() {
  pumpHumidity = pumpTempSensor.readHumidity();

  if (isnan(pumpHumidity)) {
    pumpHumidity = 0.0;
    pumpHumidityToShow = "---";
  } else {
    pumpHumidityToShow = String(pumpHumidity, 0);       
  }

  compareValues(5, pumpHumidityToShow, lastPumpHumidityToShow);

  if (pumpHumidity > 90) isTooHighHumidity = true;
  else if (pumpHumidity == 0) isTooHighHumidity = true;
  else isTooHighHumidity = false;
}

void checkPumpWithoutWater() {
  if (digitalRead(pinPumpWithoutWater) == LOW) {
    // Čerpadlo je bez vody
    if (pumpWithoutWaterTimeActivation > 0) {
      if ((millis() - pumpWithoutWaterTimeActivation) > delayAfterPumpWithoutWaterMillis) {
        isPumpWithoutWater = true;
        pumpWithoutWaterTimeActivation = 0;
      }
    } else {
      pumpWithoutWaterTimeActivation = millis();
    }
  } else {
    if (isPumpWithoutWater) {
      if (pumpWithoutWaterTimeActivation > 0) {
        if ((millis() - pumpWithoutWaterTimeActivation) > delayAfterPumpWithoutWaterMillis) {
          isPumpWithoutWater = false;
          pumpWithoutWaterTimeActivation = 0;
        }
      } else {
        pumpWithoutWaterTimeActivation = millis();
      }
    } else {
      isPumpWithoutWater = false;
      pumpWithoutWaterTimeActivation = 0;
    }
  }

  updateCharNoWaterAlert();
}

void checkPumpInWater() {
  // Sepnutý hladinový spínač?
  if (digitalRead(pinPumpInWater) == LOW) {
    //Již tato událost byla zachycena dříve
    if (isPumpInWater == true) {
      return;
    } else {
      isPumpInWater = true;
      pumpInWaterTimeActivation = millis();
    }
  } else {
    //Právě došlo k poklesu hladiny po předchozím zaplavení prostoru čerpadla
    //Po poklesu hladiny po zaplavení musí být čerpadlo ještě nějakou dobu vypnuté (vysušení)
    if (isPumpInWater) {
      isPumpInWater = false;
      pumpInWaterTimeActivation = 0;
      isPumpWet = true;
      waterLevelDropTime = millis();  //Nastavení času začátku sušení
      return;
    }

    //Je čerpadlo mokré? (Po zaplavení prostoru čerpadla a po pominutí zaplavení)
    if (isPumpWet) {
      checkPumpDrying();
    }
  }
}

void checkPumpDrying() {
  if ((millis() - waterLevelDropTime) > delayDryingMillis) {
    isPumpWet = false;
    waterLevelDropTime = 0;
  }
}

void checkTemperatureInOut() {
  if (canMeasureInOutTemperature() == false) {
    return;
  }
  
  sensorsInOut.requestTemperatures();

  tempWaterIn = sensorsInOut.getTempCByIndex(0);
  tempWaterOut = sensorsInOut.getTempCByIndex(1);

  if (isnan(tempWaterIn) || tempWaterIn < 0) {
    tempWaterIn = 0.0;
    tempWaterInToShow = "---";
  } else {
    tempWaterInToShow = String(tempWaterIn, 1);
  }
  
  if (isnan(tempWaterOut) || tempWaterOut < 0) {
    tempWaterOut = 0.0;
    tempWaterOutToShow = "---";
  } else {
    tempWaterOutToShow = String(tempWaterOut, 1);
  }

  compareValues(1, tempWaterInToShow, lastTempWaterInToShow);
  compareValues(2, tempWaterOutToShow, lastTempWaterOutToShow);
}

void checkTemperaturePool() {
  if (canMeasurePoolTemperature() == false) {
    return;
  }
  
  sensorPool.requestTemperatures();
  tempWaterPool = sensorPool.getTempCByIndex(0);

  if (isnan(tempWaterPool) || tempWaterPool < 0) {
    tempWaterPool = 0.0;
    tempWaterPoolToShow = "---";
  } else {
    tempWaterPoolToShow = String(tempWaterPool, 1);
  }

  compareValues(3, tempWaterPoolToShow, lastTempWaterPoolToShow);
  
  if (isOnlyFilter) return;

  if (tempWaterPool > 28) {
    hasMaxTemp = true;
  } else {
    if (tempWaterIn <= 25) hasMaxTemp = false;
  }
}

void checkOnlyFilter() {
  int pinValue = digitalRead(pinModeOnlyFilter);

  if (lastOnlyFilterSwitchMoved > 0) {
    if ((millis() - lastOnlyFilterSwitchMoved) < delayForOnlyFilterSwitchMillis) {
      return;
    }

    lastOnlyFilterSwitchMoved = 0;
    lastIsOnlyFilter = pinValue;
  }

  //Změna stavu tlačítka - počkáme než zakmitá
  if (lastIsOnlyFilter != pinValue) {
    lastOnlyFilterSwitchMoved = millis();
    return;
  }

  if (pinValue == HIGH) {
    if (!isOnlyFilter) {
      isOnlyFilter = true;
      lastIsOnlyFilter = true;
      hasMaxTemp = false;
    }
  } else {
    if (isOnlyFilter) {
      isOnlyFilter = false;
      lastIsOnlyFilter = false;
    }
  }
}

//Přivede impuls na zapínací relé
void startSsrRelay(boolean start) {
  if (start) {
    if (pumpIsRunning) return;
    if (isManualOff) return;

    digitalWrite(pinRelayOn, HIGH);
    pumpIsRunning = true;
    deviceStartet = true;
    digitalWrite(pinLed, HIGH);
  } else {
    if (!pumpIsRunning) return;

    digitalWrite(pinLed, LOW);
    digitalWrite(pinRelayOn, LOW);
    pumpIsRunning = false;
    deviceStartet = false;
  }
}

void beep() {
  digitalWrite(pinBeep, HIGH);
  delay(delayBeepMillis);
  digitalWrite(pinBeep, LOW);
}

void checkVentRequire() {
  if (isPumpInWater) {
    startPumpVent(false);
    return;
  }

  // Chlazení poběží trvale
  startPumpVent(true);
  return;

  if (isPumpWet || isPumpWarm || isPumpHot || isTooHighHumidity) {
    startPumpVent(true);
  } else {
    startPumpVent(false);
  }
}

void startPumpVent(boolean start) {
  if (start) {
    if (pumpVentIsRunning) return;
    digitalWrite(pinPumpVent, LOW);
    pumpVentIsRunning = true;
  }
  else {
    if (!pumpVentIsRunning) return;
    digitalWrite(pinPumpVent, HIGH);
    pumpVentIsRunning = false;
  }
}

void waitForStop() {
  for (waitForStopCounter = 3; waitForStopCounter > 0; waitForStopCounter --) {
    
    if (isRequestStopAfterTurningOn) {
      isManualOff = true;
      waitForShowMessage();    
      break;
    } else {
      updateWaitCounter();
      delay(1000);
    }
  }
}

void waitForShowMessage() {
  for (waitForStopCounter = 5; waitForStopCounter > 0; waitForStopCounter --) {
    updateWaitCounter();
    delay(1000);
  }  
}

void backlight(boolean on) {
  if (on) {
    displayBackLightOnTimeActivation = millis();
    if (isBackLight) return;
    isBackLight = true;
    digitalWrite(pinDysplayBackLight, HIGH);
  } else {
    if (!isBackLight) return;
    displayBackLightOnTimeActivation = 0;
    isBackLight = false;
    digitalWrite(pinDysplayBackLight, LOW);
  }
}

boolean canMeasurePoolTemperature() {
  if ((millis() - lastPoolTemperatureMeasurement) > delayForPoolTemperatureMeasurement) {
    lastPoolTemperatureMeasurement = millis();
    return true;
  } else {
    return false;
  }
}

boolean canMeasureInOutTemperature() {
  if ((millis() - lastInOutTemperatureMeasurement) > delayForInOutTemperatureMeasurement) {
    lastInOutTemperatureMeasurement = millis();
    return true;
  } else {
    return false;
  }
}

void updateStatusBar() {
  updateCharPumpIsRunning();
  updateCharPumpVentIsRunning();
  updateCharTempWaterAlert();
  updateCharNoWaterAlert();
  updateCharPumpInWaterAlert();
  updateCharPumpWetAlert();
  updateCharPumpHotAlert();
  updateCharHumidytyPumpAlert();
  updateManualOffAlert();
  updateOnlyFilterAlert();
}

void updateCharPumpIsRunning() {
  lcd.setCursor(0, 3);
  if (pumpIsRunning == 1) lcd.write((char)126);
  else lcd.print(" ");
}

void updateCharPumpVentIsRunning() {
  lcd.setCursor(1, 3);
  if (pumpVentIsRunning == 1) lcd.write(byte(charVentAlertIndex));
  else lcd.print(" ");
}

void updateCharTempWaterAlert() {
  lcd.setCursor(2, 3);
  if (hasMaxTemp == 1) lcd.write(byte(charTempWaterAlertIndex));
  else lcd.print(" ");
}

void updateCharNoWaterAlert() {
  lcd.setCursor(4, 3);
  if (isPumpWithoutWater == 1) lcd.write(byte(charNoWaterAlertIndex));
  else lcd.print(" ");
}

void updateCharPumpInWaterAlert() {
  lcd.setCursor(5, 3);
  if (isPumpInWater == 1) lcd.write(byte(charPumpInWaterAlertIndex));
  else lcd.print(" ");
}

void updateCharPumpWetAlert() {
  lcd.setCursor(6, 3);
  if (isPumpWet == 1) lcd.write(byte(charPumpWetAlertIndex));
  else lcd.print(" ");
}

void updateCharPumpHotAlert() {
  lcd.setCursor(8, 3);
  if (isPumpHot == 1) lcd.write("H");
  else lcd.print(" ");
}

void updateCharHumidytyPumpAlert() {
  lcd.setCursor(9, 3);
  if (isTooHighHumidity == 1) lcd.write("V");
  else lcd.print(" ");
}

void updateManualOffAlert() {
  lcd.setCursor(10, 3);
  if (isManualOff == 1) lcd.write("M");
  else lcd.print(" ");
}

void updateOnlyFilterAlert() {
  lcd.setCursor(11, 3);
  if (isOnlyFilter == 1) lcd.write("F");
  else lcd.print(" ");
}

void updatePoolTemp() {
  if (modeSelect == 0) {
    lcd.setCursor(0, 1);
    lcd.print("Teplota vody bazenu");
    lcd.setCursor(0, 2);
    lcd.print("                    ");
    lcd.setCursor(9, 2);
    lcd.print(lastTempWaterPoolToShow);
    lcd.print((char)223);
  } else {
    lcd.setCursor(0, 0);
    lcd.print("                    ");
    lcd.setCursor(0, 0);
    lcd.print("Bazen: ");
    lcd.print(lastTempWaterPoolToShow);
    lcd.print((char)223);
  }    
}

void updateInTemp() {
  if (modeSelect == 0) return;
  
  lcd.setCursor(0, 1);
  lcd.print("        ");
  lcd.setCursor(0, 1);
  lcd.print("IN:");
  lcd.print(lastTempWaterInToShow);
  lcd.print((char)223);
}

void updateOutTemp() {
  if (modeSelect == 0) return;
  
  lcd.setCursor(11, 0);
  lcd.print("         ");
  lcd.setCursor(11, 1);
  lcd.print("OUT:");
  lcd.print(lastTempWaterOutToShow);
  lcd.print((char)223);
}

void updatePumpTemp() {
  if (modeSelect == 0) return;
  
  lcd.setCursor(0, 2);
  lcd.print("tC:");
  lcd.print("     ");
  lcd.setCursor(3, 2);
  lcd.print(lastPumpTemperatureToShow);
  lcd.print((char)223);
}

void updatePumpHumidity() {
  if (modeSelect == 0) return;
  
  lcd.setCursor(10, 2);
  lcd.print("vC:");
  lcd.print("     ");
  lcd.setCursor(13, 2);
  lcd.print(lastPumpHumidityToShow);
  lcd.print("%");
}

/*
Konstanty pro rozlišení typů hodnot
 tempWaterIn = 1
 tempWaterOut = 2
 tempWaterPool = 3
 pumpTemperature = 4
 pumpHumidity = 5
*/
void compareValues(byte valueType, String value1, String value2) {
  if (value1 != value2) {
    switch (valueType) {
      case 1:
        lastTempWaterInToShow = tempWaterInToShow;
        updateInTemp();
        break;
      case 2:
        lastTempWaterOutToShow = tempWaterOutToShow;
        updateOutTemp();
        break;
      case 3:
        lastTempWaterPoolToShow = tempWaterPoolToShow;
        updatePoolTemp();
        break;
      case 4:
        lastPumpTemperatureToShow = pumpTemperatureToShow;
        updatePumpTemp();
        break;
      case 5:
        lastPumpHumidityToShow = pumpHumidityToShow;
        updatePumpHumidity();
        break;
    }
  }
}

void updateWaitCounter() {
  if (isRequestStopAfterTurningOn) {
    lcd.clear();

    //Odpočet času, po který je zobrazena tato zpráva
    lcd.setCursor(9, 0);
    lcd.print(String(waitForStopCounter));
    
    lcd.setCursor(2, 1);
    lcd.print("Cerpadlo vypnuto");
    lcd.setCursor(0, 3);

    if (waitForStopCounter == 4) {
      lcd.print("Necum na meeeee!");
    } else if (waitForStopCounter == 3) {
      lcd.print("Je mi to uplne jedno");
    } else if (waitForStopCounter == 2) {
      lcd.print("Hmmm Hmmmm");
    } else if (waitForStopCounter == 1) {
      lcd.print(":-D :-D :-D");
    }
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Cerpadlo se zapne za");
    lcd.setCursor(8, 1);
    lcd.print("  ");
    lcd.setCursor(8, 1);
    lcd.print(waitForStopCounter);
    lcd.setCursor(0, 3);
    lcd.print("Start zrus tlacitkem");
  }
}
