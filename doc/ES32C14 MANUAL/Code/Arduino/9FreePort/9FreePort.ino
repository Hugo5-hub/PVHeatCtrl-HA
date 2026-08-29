//Author: eletechsup
//More information welcome to : http://www.485io.com 
//Arduino IDE 2.2.1
//ES32C14 Expansion Board for ESP32 38PIN BOARD
//There are 9 free ports(IO33/IO32/IO15/IO2/IO0/IO4/IO16/IO21/IO23) for IO expansion
//This routine function: 9 free ports, control 4 relay outputs


  
//9 free ports as Input control 4 relay outputs
const int IO33_Port     =   33;
const int IO32_Port    =   32;
const int IO15_Port    =   15;
const int IO2_Port     =   2;
const int IO0_Port    =    0;
const int IO4_Port    =    4;
const int IO16_Port    =   16;
const int IO21_Port    =   21;
const int IO23_Port    =   23;

const int CH1 = 27;   
const int CH2 = 14;  
const int CH3 = 12;   
const int CH4 = 13;


void setup() {                 

  pinMode(IO33_Port, INPUT_PULLUP); 
  pinMode(IO32_Port, INPUT_PULLUP); 
  pinMode(IO15_Port, INPUT_PULLUP); 
  pinMode(IO2_Port, INPUT_PULLUP); 
  pinMode(IO0_Port, INPUT_PULLUP); 
  pinMode(IO4_Port, INPUT_PULLUP); 
  pinMode(IO16_Port, INPUT_PULLUP); 
  pinMode(IO21_Port, INPUT_PULLUP);
  pinMode(IO23_Port, INPUT_PULLUP);  

  pinMode(CH1, OUTPUT);    
  pinMode(CH2, OUTPUT);    
  pinMode(CH3, OUTPUT);    
  pinMode(CH4, OUTPUT); 

}
  
void loop() { 

 if (digitalRead(IO33_Port) == 0) {
    digitalWrite(CH1, HIGH);
  } else {
    digitalWrite(CH1, LOW);
  }
  if (digitalRead(IO32_Port) == 0) {
    digitalWrite(CH2, HIGH);
  } else {
    digitalWrite(CH2, LOW);
  }
  if (digitalRead(IO15_Port) == 0) {
    digitalWrite(CH3, HIGH);
  } else {
    digitalWrite(CH3, LOW);
  }
  if (digitalRead(IO2_Port) == 0) {
    digitalWrite(CH4, HIGH);
  } else {
    digitalWrite(CH4, LOW);
  }

}
