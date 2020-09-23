const int cidloPin = 2;
const int pinLed = 4;

long detectStart = 0;
long detectDelay = 0;

void setup() {
  Serial.begin(9600);  
  pinMode(cidloPin, INPUT);
  pinMode(pinLed, OUTPUT);
  
  digitalWrite(pinLed, LOW);
  
  attachInterrupt(0, detekce, CHANGE);
}

void loop() {
  delay(1000);
}

void detekce() {
  if (digitalRead(cidloPin) == HIGH) {
    digitalWrite(pinLed, HIGH);
    detectStart = millis();
    Serial.println("--- START ---");
  } else {
    digitalWrite(pinLed, LOW);
    Serial.println("--- STOP ---");
    
    detectDelay = (millis() - detectStart) / 1000;

    Serial.print(String(detectDelay));
    Serial.println("s");
  }
}
