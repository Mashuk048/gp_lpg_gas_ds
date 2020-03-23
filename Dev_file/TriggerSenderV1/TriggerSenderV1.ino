#include <RCSwitch.h>
RCSwitch mySwitch = RCSwitch();


unsigned long lastMsgTime = 0;
const long INTERVAL = 20000;
int D3=0; //added for my ide


void setup() {

  Serial.begin(115200);

  Serial.println("TX READY");

  delay(500);
  // Transmitter is connected to Arduino Pin #10  
  mySwitch.enableTransmit(D3); //gpio 0
  
  // Optional set protocol (default is 1, will work for most outlets)
  // mySwitch.setProtocol(2);

  // Optional set pulse length.
  // mySwitch.setPulseLength(320);
  
  // Optional set number of transmission repetitions.
  // mySwitch.setRepeatTransmit(15);
  
}

void loop() {
  
  setInterval();
  
}


void setInterval(){
  
  unsigned long now = millis();
  
  if(now - lastMsgTime > INTERVAL){
    Serial.println("Ticking every 5 seconds");
    lastMsgTime = now;    
    
    mySwitch.send(5555, 24);
//    delay(1000);
//    mySwitch.send(5396, 24);
//    delay(1000);
  }
  
}
