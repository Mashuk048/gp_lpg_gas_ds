float temp() {
  
  Serial.print("Temp +");
  for (int i = 1; i <= 10; i++) {
    float t = dht.readTemperature();
    Temperature = Temperature + t;
    delay(75);
  }
  Temperature = (Temperature / 10);
  return Temperature ;
}

float hum() {
  float h = dht.readHumidity();
   Serial.print(" hum +");
  for (int i = 1; i <= 10; i++) {
    Humidity = Humidity + h;
    delay(75);
  }
  Humidity = (Humidity / 10) ;
  return Humidity;
}



int tgsVal() {
  Serial.print(" TGS_Val");
  
  for (int i = 1; i <= 10; i++) {
    int raw = analogRead(analogPin);
    methane = pow(10, .00390625 * (raw - 0));
    delay(75);
  }
  methane = methane / 10;
  return methane;
}
