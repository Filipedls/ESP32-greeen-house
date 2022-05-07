#define ELEMENTS(x)   (sizeof(x) / sizeof(x[0]))
//jj
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
  // wait 3s for the temp sensor
  delay(3000);
}

unsigned long previousMillis = 0;
const long interval = 60000;//2*60000; // 1 mins (in ms)

void loop(){
  
  for(int i=1; i<21;i++){
    previousMillis = millis();
    
    float temp = NAN;
    float humid = NAN;
    int fan_Speed;
    readDHTTemperatureHumidity(&temp, &humid);
    
    updateStage(temp);
    int fan_speed = updateFanSpeed(temp);
    updateTempHumidAvg(temp, humid); 
        
//    if(i==1){
//      getResetTemperatureHumidityAvg(&temp, &humid);
//      Serial.println("Avg temp/humid avg - "+String(temp)+" / "+String(humid));
//    }
    if(i==1){
      int avg_light_power = getAvgLightPower();
      logTempHumidToGS(true, fan_speed, avg_light_power);
    }

    while (millis() <= previousMillis + interval);
    //Serial.println("LOOP! Passed: "+String(millis()-startMillis)+" ms");
  }
}
