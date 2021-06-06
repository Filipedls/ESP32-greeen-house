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
  setupLogs();
  setupStages(); // needs setupTime
  setupServer(); // needs setupTime, setupStages
  // wait 10s for the temp sensor
  delay(10000);
  logTempHumidToGS();
  delay(5000);
}
 
void loop(){
  for(int i=1; i<6;i++){
    setFanSpeedFromStage();
    if(i==5)
      logTempHumidToGS();
      
    delay(3*60000);
  }
}
