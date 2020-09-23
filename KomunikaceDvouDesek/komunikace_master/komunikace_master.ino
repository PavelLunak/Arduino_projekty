#include <Wire.h>

//master

char a = ' ';

void setup(){
    Wire.begin();
    Serial.begin(9600);
}

void loop(){
    while(Serial.available() > 0){
        a = Serial.read();
    }
    
    if(a != ' '){
        Wire.beginTransmission(100);
        Wire.write(a);
        Wire.endTransmission();
    }
    
    a = ' ';
    delay(500);
}
