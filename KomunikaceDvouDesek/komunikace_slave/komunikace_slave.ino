#include <Wire.h>

char a;

void setup(){
    Wire.begin(100);   
    pinMode(13, OUTPUT);    
    Wire.onReceive(priPrijmu);
}

void loop(){
    delay(100);
    if(a == 'H'){
        digitalWrite(13, HIGH);    
    }
    else if(a == 'L'){
        digitalWrite(13, LOW);    
    }
}

void priPrijmu(int b){
    while(Wire.available() > 0){
        a = Wire.read();
    }
}
