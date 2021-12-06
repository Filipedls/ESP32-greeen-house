#define ELEMENTS(x)   (sizeof(x) / sizeof(x[0]))

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);

  // Setup's  `x)
  setupEEPROM();
  setupPWM();
  setupDHT();
  setupWiFi();
  setupTime(); // needs setupWiFi
  //setupLogs();
  setupStages(); // needs setupTime; runs setupEEPROM (make to run any read after)
  setupFan(); 
  setupServer(); // needs setupTime, setupStages
  // wait 5s for the temp sensor
  delay(5000);
}

//unsigned long startMillis = 0;
unsigned long previousMillis = 0;
//unsigned long currentMillis = 0;
const long interval = 60000;//2*60000; // 2 mins (in ms)

void loop(){
  
//  unsigned long currentMillis = millis();
//  if (currentMillis - previousMillis >= interval) {
//    // save the last time you blinked the LED
//    previousMillis = currentMillis;
//
//  }
//  startMillis = millis();
  for(int i=1; i<21;i++){
    previousMillis = millis();
    
    updateFanSpeed();
//    if(i==1)
//      logTempHumidToGS();
//      
    //delay(interval);
    //delay( max(interval-(millis()-previousMillis), (unsigned long)(0) ) );
    while (millis() < previousMillis + interval);
    //Serial.println("LOOP! Passed: "+String(millis()-startMillis)+" ms");
  }
}
