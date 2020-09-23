#include <Wire.h>
#include <SPI.h>
#include <Ethernet.h>

#define delaySettingsLoad 20000                     //20 vteřin - interval stahování nastavení ze serveru

unsigned long lastSettingsLoad = 0;                 //Čas posledního pokusu stažení nstavení
unsigned long lastSendDataToServer = 0;             //Čas posledního pokusu o odeslání dat
unsigned long remoteSendInterval = 10000;           //10 vteřin - interval odesílání dat na server

byte remoteControllOn = 0;
byte remotePumpOn = 0;
byte remoteOnlyFilter = 0;

byte isMessageReceived = 0;
byte messageCode;

byte tempWaterPool = 0;
byte tempWaterIn = 0;
byte tempWaterOut = 0;
byte lightIntensityInPercentages = 0;
byte pumpTemperature = 0;
byte pumpHumidity = 0;

byte tempWaterInTenths = 0;
byte tempWaterOutTenths = 0;
byte tempWaterPoolTenths = 0;

byte pumpIsRunning = false;
byte pumpVentIsRunning = false;
byte hasMaxTemp = false;
byte isCloudy = false;
byte isPumpWithoutWater = false;
byte isPumpInWater = false;
byte isPumpWet = false;
byte isPumpWarm = false;
byte isPumpHot = false;
byte isTooHighHumidity = false;
byte isManualOff = false;
byte isOnlyFilter = false;



byte mac[] = {0x90, 0xA2, 0xDA, 0x00, 0x9C, 0xB7};
IPAddress ip(10, 0, 0, 55);
char server[] = "www.mrazenapizzaexpress.cz";
EthernetClient client;

void setup() {
  Serial.begin(9600);

  if (Ethernet.begin(mac) == 0) {
    Ethernet.begin(mac, ip);
  }

  Wire.begin(3);
  Wire.onRequest(sendData);
}

void loop() {
  checkLastSettingsLoad();
  checkLastDataSend();
  sendEventToServer(0);
  delay(5000);
}

void checkLastSettingsLoad() {
  if ((millis() - lastSettingsLoad) > delaySettingsLoad) {
    lastSettingsLoad = millis();
    getSettingsFromServer();
  }
}

void checkLastDataSend() {
  //Serial.println("checkLastDataSend()");
  if ((millis() - lastSendDataToServer) > remoteSendInterval) {
    lastSendDataToServer = millis();
    sendDataToServer();
  }
}

void sendDataToServer() {
  Serial.println("sendDataToServer()");

  if (client.connect(server, 80)) {
    Serial.println("client OK");
    delay(1000);

    client.println((String)"GET /Bazen/api.php?"
                   + "new_data=1"
                   + "&teplota_bazenu=" + (float) (tempWaterPool + (float)(tempWaterPoolTenths / 10))
                   + "&teplota_cerpadla=" + pumpTemperature
                   + "&vlhkost_cerpadla=" + pumpHumidity
                   + "&teplota_in=" + (float) (tempWaterIn + (float)(tempWaterInTenths / 10))
                   + "&teplota_out=" + (float) (tempWaterOut + (float)(tempWaterOutTenths / 10))
                   + "&intenzita_svetla=" + lightIntensityInPercentages
                   + "&sensorIsPumpWithoutWater=" + isPumpWithoutWater
                   + "&sensorIsPumpInWater=" + isPumpInWater
                   + "&sensorIsPumpWet=" + isPumpWet
                   + "&sensorIsPumpWarm=" + isPumpWarm
                   + "&sensorIsPumpHot=" + isPumpHot
                   + "&isPumpWentRunning=" + pumpVentIsRunning
                   + "&isPumpRunning=" + pumpIsRunning
                   + " HTTP/1.1");

    client.println("Host: www.mrazenapizzaexpress.cz");
    client.println("Connection: close");
    client.println();

    delay(100);

    while (client.available()) {
      char c = client.read();
      Serial.print(c);
    }

    client.stop();
  } else {
    Serial.println("client ERROR");
  }
}

void sendEventToServer(byte type) {
  Serial.println("sendEventToServer");
  if (client.connect(server, 80)) {
    Serial.println("client OK");
    delay(1000);

    client.println((String)"GET /Bazen/api.php?"
                   + "new_udalost=1"
                   + "&typ=" + random(0, 18)
                   + " HTTP/1.1");

    client.println("Host: www.mrazenapizzaexpress.cz");
    client.println("Connection: close");
    client.println();

    delay(100);

    while (client.available()) {
      char c = client.read();
      Serial.print(c);
    }

    client.stop();
  }
}

void getSettingsFromServer() {
  Serial.println("getSettingsFromServer()");

  if (client.connect(server, 80)) {

    Serial.println("client OK");

    char buffer[255]; //zásobník na příchozí řetězec

    delay(1000);

    client.println("GET /Bazen/api.php?nastaveni_arduino=1 HTTP/1.1");
    client.println("Host: www.mrazenapizzaexpress.cz");
    client.println("Connection: close");
    client.println();

    delay(100);

    char resultData[6];
    parseResponse(resultData, 6);
    Serial.println(resultData);

    for (int i = 0; i < sizeof(resultData); i ++) {
      Serial.print(resultData[i]);
      if (i < 5) Serial.print("-");
    }

    Serial.println("");

    client.stop();
  } else {
    Serial.println("client ERROR");
  }
}

int parseResponse(char result[], int size) {
  // skip HTTP/1.1
  if (!client.find("HTTP/1.1")) {
    return -1;
  }

  int st = client.parseInt(); // parse status code
  int l = -1;

  if (st == 200 && client.find("resultdata")) {
    int l = client.readBytesUntil('"', result, size);
  }

  return l;
}

void sendData() {
  Wire.beginTransmission (3);

  Wire.write(remoteControllOn);
  Wire.write(remotePumpOn);
  Wire.write(remoteOnlyFilter);
  Wire.write(remoteSendInterval);

  Wire.endTransmission ();
}

void receiveData(int howMany) {
  isMessageReceived = Wire.read();

  if (isMessageReceived == 1) {
    messageCode = Wire.read();
    sendEventToServer(messageCode);
  } else {
    tempWaterPool = Wire.read();
    tempWaterPoolTenths = Wire.read();

    tempWaterIn = Wire.read();
    tempWaterInTenths = Wire.read();

    tempWaterOut = Wire.read();
    tempWaterOutTenths = Wire.read();

    lightIntensityInPercentages = Wire.read();

    pumpTemperature = Wire.read();
    pumpHumidity = Wire.read();

    pumpIsRunning = Wire.read();
    pumpVentIsRunning = Wire.read();

    hasMaxTemp = Wire.read();
    isCloudy = Wire.read();
    isPumpWithoutWater = Wire.read();
    isPumpInWater = Wire.read();
    isPumpWet = Wire.read();
    isPumpWarm = Wire.read();
    isPumpHot = Wire.read();
    isTooHighHumidity = Wire.read();
    isManualOff = Wire.read();
    isOnlyFilter = Wire.read();
  }
}
