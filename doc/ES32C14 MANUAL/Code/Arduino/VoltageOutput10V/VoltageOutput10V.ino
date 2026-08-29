//Author: eletechsup
//More information welcome to : http://www.485io.com 
//Arduino IDE 2.2.1
//ES32C14 Expansion Board for ESP32 38PIN BOARD

/*
Pay attention to the status of the DIP switch
SW1:
1-OFF
2-OFF
3-ON
4-ON
*/

#define Vo1 25   //Vo1 DAC1 IO25
#define Vo2 26   //Vo2 DAC2 IO26

void setup() {
  dacWrite(Vo1, 102);    //4V
  dacWrite(Vo2, 205);    //8V
}

void loop() {
  
}
