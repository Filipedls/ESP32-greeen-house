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
  //setupLogs();
  setupStages(); // needs setupTime
  setupServer(); // needs setupTime, setupStages
  // wait 5s for the temp sensor
  delay(5000);
}
 
void loop(){
  for(int i=1; i<6;i++){
    updateFanSpeed();
    if(i==1)
      logTempHumidToGS();
      
    delay(4*60000);
  }
}
