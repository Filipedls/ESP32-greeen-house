#define MAX_TEMPS_SIZE 10
struct TempCfg {
    char tname[20];
    int tempsSize;
    int temps[MAX_TEMPS_SIZE];
    int fanSpeeds[MAX_TEMPS_SIZE];
    float lightPowerRatio;
};

// absolute min fan speed, bellow this the fan will stop
const int abs_min_fan_speed = 45;
// setable min_fan_speed
int min_fan_speed = abs_min_fan_speed;

#define MAX_PWM_FAN 255.0

// for noise reason the temp control will keep the fan speed at
// max_fan_speed until no_max_fan_speed_temp is reached
int max_fan_speed = int(MAX_PWM_FAN);
// above 30C ignores the limit on the fan speed
int no_max_fan_speed_temp = 30;

bool max_fan_speed_warn = false;

// temp config  for all stages
TempCfg main_temp_config = { 
    // FAN
    //tname
    "main",
    // tempsSize
    4,
    // temps
    {20,27 ,30 ,33 },
    // fanSpeeds
    {0 ,100,210,255},
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

// pid defined by having no temp profile and a lightPowerRatioWeight bellow 1
TempCfg pid_temp_config = {"PID", 0,{},{}, 0.95};

// proportional to the avg of the lights
TempCfg light_temp_config = {"light", 0,{},{}, 1.0};

TempCfg off_temp_config = {"off", 0,{},{},-1.0};

#define NTEMPCONFIGS 5
TempCfg * all_temp_configs[NTEMPCONFIGS] {&main_temp_config, &dynamic_temp_config, &pid_temp_config, &light_temp_config, &off_temp_config};
int selTempCfg = 0;

// FAN //

bool isFanModeOFF(){
  return selTempCfg == NTEMPCONFIGS-1;
}

void setMainFanPwm(int val){
  if(isFanModeOFF()){
    // when the fan speed is manually set the warn is cleared
    if(max_fan_speed_warn) max_fan_speed_warn = false;
    if(val < (MAX_PWM_FAN/2)){
      min_fan_speed = max(val, abs_min_fan_speed);
      Serial.println("setMainFanPwm: setting min fan speed to" + String(min_fan_speed) );
    } else {
      max_fan_speed = val;
      Serial.println("setMainFanPwm: setting max fan speed to" + String(max_fan_speed) );
    }
  } else
    // checks the set min
    val = max(val, min_fan_speed);
    
  setPwmVal(NLIGHTS, val);
}

void restoreFanMode(){
  // initialize EEPROM with predefined size
  //EEPROM.begin(EEPROM_T_SIZE);
  int mid_fan_speed_temp_mem = getMemFanModeTemp();
  if(mid_fan_speed_temp_mem > 33 || mid_fan_speed_temp_mem < 20)
    mid_fan_speed_temp_mem = mid_fan_speed_temp;
  propagateDynamicTemp(mid_fan_speed_temp_mem);
  // fand mode
  int selTempCfg_mem = getMemFanModeNum();
  if(selTempCfg_mem < NTEMPCONFIGS && selTempCfg_mem >= 0)
    selTempCfg = selTempCfg_mem;
}

void propagateDynamicTemp(int mid_fan_speed_temp_new_val){
  mid_fan_speed_temp = mid_fan_speed_temp_new_val;
  dynamic_temp_config.temps[0] = mid_fan_speed_temp-temp_offset;
  dynamic_temp_config.temps[1] = mid_fan_speed_temp+temp_offset;
}

void setDynamicTemp(int mid_fan_speed_temp_new_val){
  propagateDynamicTemp(mid_fan_speed_temp_new_val);
  if(isFanModeOFF())
    no_max_fan_speed_temp = mid_fan_speed_temp_new_val;

  float temperature = readDHTTemperature();
  updateFanSpeed(temperature);
  // when the mid_fan_speed_temp is manually set the warn is cleared
  if(max_fan_speed_warn) max_fan_speed_warn = false;
  
  setMemFanModeTemp(mid_fan_speed_temp);
}

// TIMER

//hw_timer_t * timer_fan = NULL;
//bool timer_is_ON = false;
//void stopAutoFan(){
//  // (re)start alarm to startAutoFan in 30 mins
//  timerAlarmDisable(timer_fan);
//  timerRestart(timer_fan);
//  timerAlarmWrite(timer_fan, 30*600000, false);
//  timerAlarmEnable(timer_fan);
//  
//  if(!timer_is_ON){
//    //Serial.println("fan timer going ON");
//    timer_is_ON = true;
//  } else {
//    //Serial.println("fan timer going RESTART");
//  }
//}
//
//void IRAM_ATTR startAutoFan(){
//  if(timer_is_ON){
//    //Serial.println("fan timer going OFF");
//    timer_is_ON = false;
//  } else {
//    //Serial.println("fan timer going OFF, again?!");
//  }
//}

void setTempConfig(int tempConfigN){
  selTempCfg = tempConfigN;
//  if(timer_is_ON){
//    timer_is_ON = false;
//  }
  setMemFanModeNum(selTempCfg);
  float temperature = readDHTTemperature();
  updateFanSpeed(temperature);
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

// PID fan control

// config
float kI = 0.03;
float kD = 0.1;
float kP = 0.4;
int dt = 1; // 1 min?
float kI_decay = 0.01;

// aux vars
bool prev_cycle_was_PID = false;
float prev_err = 0;
float err_sum = 0;
float err = 0;
int setPIDfanSpeed(float temp, float lightPowerRatio){

  if(!prev_cycle_was_PID){
    prev_err = 0;
    err_sum = 0;
    err = 0;
    prev_cycle_was_PID = true;
  }
  int lightPowerAvg = getAvgLightPower();
  
  // mid_fan_speed_temp is the temo goal for the PID
  err = temp - mid_fan_speed_temp;
  
  // to remove the lag from the integral part, we clip the err_sum at zero and decay it
  // otherwise takes to long to react
  err_sum = max(err_sum*(1-kI_decay) + err,float(0.0));

  // a ratio of 1 mean the PID val is 100% propotional to the avg_light_power, 0 doesnt care about lights
  float power_coef = lightPowerRatio*lightPowerAvg + (1-lightPowerRatio)*MAX_PWM_FAN;

  // max(err,float(0) - we dont wat the P control to be neg, cuts the Diff part off when the fan is at min speed
  int speed_val = (kP*max(err,float(0)) + kI*err_sum*dt + kD*(err-prev_err)/dt) 
  // power and min max scalings
  * power_coef * ((MAX_PWM_FAN-min_fan_speed)/MAX_PWM_FAN) + min_fan_speed;
  // speed cannot be bellow zero and above 255
  speed_val = min(max(speed_val, 0), int(MAX_PWM_FAN));
  prev_err = err;
  //Serial.println( "PID > speed:"+String(speed_val)+" err:"+String(err)+" esum:"+String(err_sum) + " pcoef:"+String(power_coef) );
  return speed_val;
}

// AUTO temp humid  control
// have a max humid and temp, where if  above fan goes up, til back normal
int processTempCfg(struct TempCfg temps_cfg, float temperature){
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
  return pwmVal;
}

int n_times_temp_nan = 0;
void updateFanSpeed(float temperature){
//  if(timer_is_ON){
//    //Serial.println("temp timer_is_ON! nothing done");
//    return;
//  }
  
  TempCfg temps_cfg = * all_temp_configs[selTempCfg];
  //Serial.println("temp csel: "+String(selTempCfg) + " N " + String(temps_cfg.tname));
  if(temps_cfg.lightPowerRatio >= 1.0){
    int default_fan_speed = getAvgLightPower(); 
    setMainFanPwm(default_fan_speed);
    prev_cycle_was_PID = false; 
  } else if(temps_cfg.tempsSize > 0 || temps_cfg.lightPowerRatio >= 0.0){
    //float temperature = readDHTTemperature();
    if(!isnan(temperature)){
      int aux_fan_speed; 
      n_times_temp_nan = 0;
      if(temps_cfg.tempsSize > 0){
        aux_fan_speed = processTempCfg(temps_cfg, temperature);
        prev_cycle_was_PID = false;
      } else {
        // if temps_cfg.tempsSize == 0 and lightPowerRatio >= 0.0 does PID
        // when no temp profile does PID 
        aux_fan_speed = setPIDfanSpeed(temperature, temps_cfg.lightPowerRatio);
      }
      if(temperature < no_max_fan_speed_temp)
        aux_fan_speed = min(max_fan_speed,aux_fan_speed);
      else if (max_fan_speed < MAX_PWM_FAN){
      // when temo goes above the no_max_fan_speed_temp limit the max_fan_speed is reset to the max
      // so that the temp control can catch up, once its not restricted anymore
        max_fan_speed = int(MAX_PWM_FAN);
        max_fan_speed_warn = true;
      }
        
      setMainFanPwm(aux_fan_speed);
    } else {
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
  } else {
    prev_cycle_was_PID = false;  
  }
}

void setupFan(){
  restoreFanMode();
//  // Setting a timer to use in startAutoFan()
//  timer_fan = timerBegin(1, 8000, true);
//  timerAttachInterrupt(timer_fan, &startAutoFan, true);
  
  // makes sure the fans starts
  // the fan can have a speed as low as 40, but can not start with that
  setPwmVal(NLIGHTS, 80);
  delay(1500);
}
