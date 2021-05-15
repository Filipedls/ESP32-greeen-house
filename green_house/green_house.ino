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

struct ModeCfg {
    int pwmVals[6];
    int hour_on;
    int hour_off;
};

//struct ModeCfg mode1 = {{0,1,2,3,4,5},20,8};


ModeCfg all_modes[2] {
    {{0,1,2,3,4,5},13,8},
    {{9,8,7,6,5,4},9,19}
};
 
void loop(){
  // delay(2000);
  int hour = getHour();

  for(int  sel_mode  = 0; sel_mode < 2;sel_mode++){
    ModeCfg selModeCfg = all_modes[sel_mode];
    if(selModeCfg.hour_on <= selModeCfg.hour_off){
      if(hour >= selModeCfg.hour_on && hour <= selModeCfg.hour_off){
        Serial.println("mode "+String(sel_mode)+" ON");
      }  else {
        Serial.println("mode "+String(sel_mode)+" OFF");
      }
    } else {
      if(hour >= selModeCfg.hour_on || hour <= selModeCfg.hour_off){
        Serial.println("mode "+String(sel_mode)+" ON");
      }  else {
        Serial.println("mode "+String(sel_mode)+" OFF");
      }
    }
  }

  
  Serial.println("H: "+String(hour));
   delay(50000);
}
