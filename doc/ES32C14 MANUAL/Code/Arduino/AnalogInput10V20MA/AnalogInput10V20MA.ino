//Author: eletechsup
//More information welcome to : http://www.485io.com 
//Arduino IDE 2.2.1
//ES32C14 Expansion Board for ESP32 38PIN BOARD

#define Vi1   36   //INA1  ADC1_CH0
#define Vi2   39   //INA2  ADC1_CH3
#define Ii1   34   //INA5  ADC1_CH6
#define Ii2   35   //INA6  ADC1_CH7


const int RS485RD = 22;

int analog_value[4];
float in_value[4];

void setup(){
  Serial.begin(115200);
  pinMode(Vi1,INPUT);
  pinMode(Vi2,INPUT);
  pinMode(Ii1,INPUT);
  pinMode(Ii2,INPUT);
  pinMode(RS485RD, OUTPUT);
  digitalWrite(RS485RD, LOW);
}

void loop() {
 
      digitalWrite(RS485RD, HIGH);
      analog_value[0] = analogRead(Vi1);   //0--4096
      analog_value[1] = analogRead(Vi2);   //0--4096
      analog_value[2] = analogRead(Ii1);   //0--4096
      analog_value[3] = analogRead(Ii2);   //0--4096

      
      in_value[0] = (float)analog_value[0] * 3300 / 4096 / 1000 * 53 / 10 + 0.6;
      in_value[1] = (float)analog_value[1] * 3300 / 4096 / 1000 * 53 / 10 + 0.6;
      in_value[2] = ((float)analog_value[2] * 3300 / 4096 / 1000 + 0.12) / 91 * 1000;
      in_value[3] = ((float)analog_value[3] * 3300 / 4096 / 1000 + 0.12) / 91 * 1000;
   

      digitalWrite(RS485RD, HIGH);
      Serial.printf("V1=%0.2fV, V2=%0.2fV, I1=%0.2fmA, I2=%0.2fmA, \n",in_value[0],in_value[1],in_value[2],in_value[3]);
      delay(1000);
      digitalWrite(RS485RD, LOW);

}
