// Import required libraries

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  // Setup's  `x)
  setupPWM();
  setupDHT();
  setupWiFi();
  setupTime(); // needs setupWiFi
  setupStages(); // needs setupTime
  setupServer(); // needs setupTime, setupStages
}
 
void loop(){
  // delay(2000);
}
