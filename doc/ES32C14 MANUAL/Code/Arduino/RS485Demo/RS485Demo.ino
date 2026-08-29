//Author: eletechsup
//More information welcome to : http://www.485io.com 
//Arduino IDE 2.2.1
//ES32C14 Expansion Board for ESP32 38PIN BOARD
//RS485 receiving and sending test

String receivedData;
const int RS485RD = 22;

void setup() {                
  Serial.begin(115200);
  Serial.setTimeout(5);
  pinMode(RS485RD, OUTPUT);
  digitalWrite(RS485RD, LOW);
}
void loop() {  
  if (Serial.available()) {
      receivedData = Serial.readString();
      digitalWrite(RS485RD, HIGH);
      Serial.print("receivedData: ");
      Serial.println(receivedData);
      delay(100);
      digitalWrite(RS485RD, LOW);
    }
}
