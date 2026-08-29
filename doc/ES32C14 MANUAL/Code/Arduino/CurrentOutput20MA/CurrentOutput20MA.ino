//Author: eletechsup
//More information welcome to : http://www.485io.com 
//Arduino IDE 2.2.1
//ES32C14Expansion Board for ESP32 38PIN BOARD

/*
Pay attention to the status of the DIP switch
SW1:
1-ON
2-ON
3-OFF
4-OFF
*/

#define Io1 25   //Io1 DAC1 IO25
#define Io2 26   //Io2 DAC2 IO26


void setup() {
  dacWrite(Io1, 127);    //10mA
  dacWrite(Io2, 255);    //20mA
}

void loop() {
  
}
