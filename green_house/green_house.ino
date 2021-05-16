// Import required libraries

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  // Setup's  `x)
  setupPWM();
  setupDHT();
  setupWiFi();
  setupTime();
  setupStages();
  setupServer();
}
 
void loop(){
  // delay(2000);
  updateStage();
  delay(600000);
}
