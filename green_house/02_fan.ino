// TODO fan based on power mode

#define MAX_TEMPS_SIZE 10
struct TempCfg {
    char tname[20];
    int tempsSize;
    int temps[MAX_TEMPS_SIZE];
    int fanSpeeds[MAX_TEMPS_SIZE];
    float lightPowerRatio;
};

// absolute min fan speed, bellow this will stop
const int abs_min_fan_speed = 45;
// setable min_fan_speed
int min_fan_speed = abs_min_fan_speed;

// temp config  for all stages
TempCfg main_temp_config = { 
    // FAN
    //tname
    "main",
    // tempsSize
    4,
    // temps
    {24,27 ,30 ,33 },
    // fanSpeeds
    {40,100,200,255},
    // lightPowerRatioWeight
    0.4
};

// the temperature for a 50% speed
// will have a 2C offset from zero to 100%
// e.g.: for a 26 (3 offset) val means the temp profile will start at 23C (0%)
//       and end at 29 (100%) 
int mid_fan_speed_temp = 26;
// offset of 2C
int temp_offset = 3;

// initialized by setDynamicTempConfig()
TempCfg dynamic_temp_config = { 
    // FAN
    //tname
    "dynamic",
    // tempsSize
    2,
    // temps
    {mid_fan_speed_temp-temp_offset, mid_fan_speed_temp+temp_offset},
    // fanSpeeds
    {0, 255},
    // lightPowerRatioWeight
    0.5
};

TempCfg off_temp_config = {"off", 0,{},{},0.0};

#define NTEMPCONFIGS 3
TempCfg * all_temp_configs[NTEMPCONFIGS] {&main_temp_config, &dynamic_temp_config, &off_temp_config};
int selTempCfg = 0;

// FAN //

void setMainFanPwm(int val){
  if(selTempCfg == NTEMPCONFIGS-1){
    min_fan_speed = max(val, abs_min_fan_speed);
    Serial.println("setMainFanPwm: setting min fan speed to" + String(min_fan_speed) );
  }
  setPwmVal(NLIGHTS, val);
}

void setDynamicTemp(int mid_fan_speed_temp_new_val){
  mid_fan_speed_temp = mid_fan_speed_temp_new_val;
  dynamic_temp_config.temps[0] = mid_fan_speed_temp-temp_offset;
  dynamic_temp_config.temps[1] = mid_fan_speed_temp+temp_offset;
  updateFanSpeed();
}

// TIMER

hw_timer_t * timer_fan = NULL;
bool timer_is_ON = false;
void stopAutoFan(){
  // (re)start alarm to startAutoFan in 30 mins
  timerAlarmDisable(timer_fan);
  timerRestart(timer_fan);
  timerAlarmWrite(timer_fan, 30*600000, false);
  timerAlarmEnable(timer_fan);
  
  if(!timer_is_ON){
    Serial.println("fan timer going ON");
    timer_is_ON = true;
  } else {
    Serial.println("fan timer going RESTART");
  }
}

void IRAM_ATTR startAutoFan(){
  if(timer_is_ON){
    Serial.println("fan timer going OFF");
    timer_is_ON = false;
  } else {
    Serial.println("fan timer going OFF, again?!");
  }
}


void setTempConfig(int tempConfigN){
  selTempCfg = tempConfigN;
  if(timer_is_ON){
    timer_is_ON = false;
  }
  updateFanSpeed();
}

int getTempConfigN(){
  return selTempCfg;
}

int getAvgLightPower(){
  pwmValsInfo pwmInfo = getPwmVals();
  
  int lightPowerAvg = 0;
  for(int i=0; i<NLIGHTS;i++){
    lightPowerAvg += pwmInfo.vals[i];
  }
  lightPowerAvg = lightPowerAvg/NLIGHTS;
  return lightPowerAvg;
}

// AUTO temp humid  control
// have a max humid and temp, where if  above fan goes up, til back normal
void processTempCfg(struct TempCfg temps_cfg, float temperature){
  int pwmVal;
  // bellow min temp
  if(temperature < temps_cfg.temps[0]){
    pwmVal = temps_cfg.fanSpeeds[0];
  // above max temp
  } else if(temperature >= temps_cfg.temps[temps_cfg.tempsSize-1]) {
    pwmVal = temps_cfg.fanSpeeds[temps_cfg.tempsSize-1];
  // temp is somewheree in thee midle
  } else {
    for(int i=1; i<temps_cfg.tempsSize;i++){
    
      if(temperature < temps_cfg.temps[i]){
        pwmVal = temps_cfg.fanSpeeds[i]-(temps_cfg.fanSpeeds[i]-temps_cfg.fanSpeeds[i-1])*
            ((temps_cfg.temps[i]-temperature)/(temps_cfg.temps[i]-temps_cfg.temps[i-1]));
        //Serial.println( String(i)+" setting: fan to"+String(pwmVal)+" T"+String(temperature) );
        break;
      }
    }
  }
  // power ratio
  if(temps_cfg.lightPowerRatio > 0.0 && temps_cfg.lightPowerRatio <= 1.0){
    int lightPowerAvg = getAvgLightPower();
    pwmVal = temps_cfg.lightPowerRatio*lightPowerAvg + (1-temps_cfg.lightPowerRatio)*pwmVal;
    //Serial.println("PT fan to"+String(pwmVal)+" T"+String(temperature) + " avg: "+ String(lightPowerAvg)  );
  }
  // checks the set min
  pwmVal = max(pwmVal, min_fan_speed);
  setMainFanPwm(pwmVal);
}

int n_times_temp_nan = 0;
void updateFanSpeed(){
  if(timer_is_ON){
    Serial.println("temp timer_is_ON! nothing done");
    return;
  }
  
  TempCfg temps_cfg = * all_temp_configs[selTempCfg];
  //Serial.println("temp csel: "+String(selTempCfg) + " N " + String(temps_cfg.tname));
  if(temps_cfg.tempsSize > 0){
    float temperature = readDHTTemperature();
    if(!isnan(temperature)){
      n_times_temp_nan = 0;
      processTempCfg(temps_cfg, temperature);
    }else {
      if(n_times_temp_nan > 2){
        // when no temp profile default to the avg light power
        int default_fan_speed = getAvgLightPower(); 
        setMainFanPwm(default_fan_speed);
      } else {
        // to not overflow the var n_times_temp_nan, only increases til 3
        Serial.println("temp was NAN, will be using AvgLightPower after 3x!");
        n_times_temp_nan++;
      }
    }
  }
}

void setupFan(){
  // Setting a timer to use in startAutoFan()
  timer_fan = timerBegin(1, 8000, true);
  timerAttachInterrupt(timer_fan, &startAutoFan, true);
  
  // makes sure the fans starts
  // the fan can have a speed as low as 40, but can not start with that
  setMainFanPwm(80);
  delay(2000);
  // default fan mode is zero
  setTempConfig(0);
}
