// Import required libraries

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  // Setup's  `x)
  setupPWM();
  setupDHT();
  setupWiFi();
  setupTime();
  setupServer();
}
 
void loop(){
  //
  int hour = getHour();
  Serial.println("H: "+String(hour));
  delay(5000);
}
