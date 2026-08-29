//Author: eletechsup
//More information welcome to : http://www.485io.com 
//Arduino IDE 2.2.1
//ES32C14 Expansion Board for ESP32 38PIN BOARD
//4 NPN DI Control 4 Relay Output

const int IN1 = 19;   
const int IN2 = 18;   
const int IN3 = 5;   
const int IN4 = 17;   
const int CH1 = 27;   
const int CH2 = 14;  
const int CH3 = 12;   
const int CH4 = 13;



void setup() {                 
  pinMode(CH1, OUTPUT);    
  pinMode(CH2, OUTPUT);    
  pinMode(CH3, OUTPUT);    
  pinMode(CH4, OUTPUT);    
  pinMode(IN1, INPUT);      
  pinMode(IN2, INPUT);      
  pinMode(IN3, INPUT);      
  pinMode(IN4, INPUT);  
  pinMode(IN1, INPUT_PULLUP);
  pinMode(IN2, INPUT_PULLUP);
  pinMode(IN3, INPUT_PULLUP);
  pinMode(IN4, INPUT_PULLUP);    
}
void loop() {  
  if (digitalRead(IN1) == 0) {
    digitalWrite(CH1, HIGH);
  } else {
    digitalWrite(CH1, LOW);
  }
  if (digitalRead(IN2) == 0) {
    digitalWrite(CH2, HIGH);
  } else {
    digitalWrite(CH2, LOW);
  }
  if (digitalRead(IN3) == 0) {
    digitalWrite(CH3, HIGH);
  } else {
    digitalWrite(CH3, LOW);
  }
  if (digitalRead(IN4) == 0) {
    digitalWrite(CH4, HIGH);
  } else {
    digitalWrite(CH4, LOW);
  }
}
