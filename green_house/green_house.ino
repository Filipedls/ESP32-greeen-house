// Import required libraries

#define ELEMENTS(x)   (sizeof(x) / sizeof(x[0]))

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
  logTempHumidToGS();
  delay(15*60000);
}
