
enum FanModeID {
    LINEAR, PID, AVGLIGHT, DRYING, OFF  
};
//enum FanModeID fan_mode = PID;

#define MAX_TEMPS_SIZE 6
struct TempCfg {
    char tname[15];
    int tempsSize;
    int temps[MAX_TEMPS_SIZE];
    int fanSpeeds[MAX_TEMPS_SIZE];
    float lightPowerRatio;
    enum FanModeID fan_mode;
};

// bellow a certain speed the fan will stop, so be carfull with this limit
// setable min_fan_speed
int min_fan_speed;

#define MAX_PWM_FAN 255.0

// for noise reason the temp control will keep the fan speed at max_fan_speed
int max_fan_speed = int(MAX_PWM_FAN);

// temp at which lights shutt off, security
int max_temp_lights; // = 34;

// the temperature for a 50% speed
// will have a 2C offset from zero to 100%
// e.g.: for a 26 (3 offset) val means the temp profile will start at 23C (0%)
//       and end at 29 (100%) 

int main_fan_goal_temp;// = 26;
// offset of 3C
int linear_temp_offset = 3;

int min_fan_goal_temp = 20;

// initialized by setLinearTempConfig()
TempCfg linear_temp_config = { 
    // FAN
    //tname
    "Linear",
    // tempsSize
    2,
    // temps
    {main_fan_goal_temp-linear_temp_offset, main_fan_goal_temp+linear_temp_offset},
    // fanSpeeds
    {0, 255},
    // lightPowerRatioWeight
    0.5,
    LINEAR
};

// pid defined by having no temp profile and a lightPowerRatioWeight bellow 1
TempCfg pid_temp_config = {"PID", 0,{},{}, 0.95, PID};

// proportional to the avg of the lights
TempCfg light_temp_config = {"Light", 0,{},{}, 1.0, AVGLIGHT};

// Drying mode
TempCfg drying_temp_config = {"Drying", 0,{},{}, 0.0, DRYING};

TempCfg off_temp_config = {"Off", 0,{},{},-1.0, OFF};

#define NTEMPCONFIGS 5
TempCfg * all_temp_configs[NTEMPCONFIGS] {&linear_temp_config, &pid_temp_config, &light_temp_config, &drying_temp_config, &off_temp_config};
int selTempCfg = 0;

// FAN //
bool isFanModeOFF(){
  return selTempCfg == NTEMPCONFIGS-1;
}

int main_fan_speed = 0;
void setMainFanPwm(int val){
  if(val != main_fan_speed){
    main_fan_speed = val;
    setPwmVal(NLIGHTS, val);
  }
}

int prev_main_fan_speed;
void manuallySetMainFanPwm(int val){
  setMemFanManualSpeed(val);
  prev_main_fan_speed = val;
  setMainFanPwm(val);
}

// min and max fan speed are only applied to auto fan speed modes
int checkMinFanSpeed(int val){
  return min(max(val, 0), max_fan_speed);
}
void setMinFanSpeed(int val){
  min_fan_speed = checkMinFanSpeed(val);
  setFanMinSpeed(min_fan_speed);
  Serial.println("setting min fan speed to" + String(min_fan_speed) );
}

int checkMaxFanSpeed(int val){
  return max(min(val, int(MAX_PWM_FAN)), min_fan_speed);
}
void setMaxFanSpeed(int val){
  max_fan_speed = checkMaxFanSpeed(val);
  setFanMaxSpeed(max_fan_speed);
  Serial.println("setting max fan speed to" + String(max_fan_speed) );
}


void restoreFanMode(){
  int main_fan_goal_temp_mem = getMemFanModeTemp();
  setLinearTempConfig(main_fan_goal_temp_mem);
  // fand mode
  selTempCfg = getMemFanModeNum();
}

void setLinearTempConfig(int main_fan_goal_temp_new_val){
  main_fan_goal_temp = main_fan_goal_temp_new_val;
  linear_temp_config.temps[0] = main_fan_goal_temp-linear_temp_offset;
  linear_temp_config.temps[1] = main_fan_goal_temp+linear_temp_offset;
}

void setMainFanGoalTemp(int main_fan_goal_temp_new_val){
  if(main_fan_goal_temp_new_val <= max_temp_lights && main_fan_goal_temp_new_val >= min_fan_goal_temp){
    setLinearTempConfig(main_fan_goal_temp_new_val);
    float temperature = readDHTTemperature();
    updateFanSpeed(temperature);
    
    setMemFanModeTemp(main_fan_goal_temp_new_val);
  }
}


void setLinearTempOffset(int linear_temp_offset_new_val){
  linear_temp_offset = linear_temp_offset_new_val;
  setLinearTempConfig(main_fan_goal_temp);
  setMemLinearTempOffset(linear_temp_offset_new_val);
  
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
  if(tempConfigN < NTEMPCONFIGS && tempConfigN >= 0){
    selTempCfg = tempConfigN;
  //  if(timer_is_ON){
  //    timer_is_ON = false;
  //  }
    setMemFanModeNum(selTempCfg);
    float temperature = readDHTTemperature();
    updateFanSpeed(temperature);
  }
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
  
  // main_fan_goal_temp is the temo goal for the PID
  err = temp - main_fan_goal_temp;
  
  // to remove the lag from the integral part, we clip the err_sum at zero and decay it
  // otherwise takes to long to react
  // limits the error to make the integral part have a max of 1
  err_sum = min(max(err_sum*(1-kI_decay) + err,float(0.0)), float(1.0/kI));

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
int getLinearTempSpeed(struct TempCfg temps_cfg, float temperature){
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

// Drying Mode control
float perc_time_on;
int fan_period_mins;// = 10;
int prev_state = -1;
void processDryingMode(){

  int hour, mins;
  getHourMin(&hour, &mins);
  
  int min_rest = mins % fan_period_mins;
  int trans_mins = fan_period_mins * perc_time_on;
  
  if(min_rest < trans_mins && prev_state != 1){
    // fan is on
    prev_state = 1;
    setMainFanPwm(prev_main_fan_speed);
  } else if(min_rest >= trans_mins && prev_state != 0){
    // fan is off
    prev_state = 0;
    prev_main_fan_speed = main_fan_speed;
    setMainFanPwm(0);
  } 
  //Serial.println("processDryingMode >> min_rest:"+String(min_rest)+" trans_mins: "+String(trans_mins)+" prev_state: "+String(prev_state));
  
}

void setFanPeriodMins(int val_mins){
  if(60%val_mins==0 && val_mins<60){
    fan_period_mins = val_mins;
    setMemFanPeriodMins(val_mins);
  } 
}

void setPercTimeOn(int val_perc){
  float val_ratio = min(max(0, val_perc), 100) / 100.0;
  perc_time_on = val_ratio;
  Serial.println("setPercTimeOn >> "+String(val_ratio));
  setMemFanDryPercTimeOn(val_ratio);
}


int n_times_temp_nan = 0;
void updateFanSpeed(float temperature){
//  if(timer_is_ON){
//    //Serial.println("temp timer_is_ON! nothing done");
//    return;
//  }
  int fan_speed;
  TempCfg temps_cfg = * all_temp_configs[selTempCfg];
  //Serial.println("updateFanSpeed >> #"+String(selTempCfg)+" Name: "+String(temps_cfg.tname)+" Mode: "+String(temps_cfg.fan_mode));
//  if(temps_cfg.fan_mode == LINEAR) {
//    Serial.println("LINEAR");
//  } else if(temps_cfg.fan_mode == PID) {
//    Serial.println("PID");
//  }
  if(temps_cfg.fan_mode == AVGLIGHT){
    fan_speed = getAvgLightPower(); 
    setMainFanPwm(fan_speed);
    prev_cycle_was_PID = false; 
  //} else if(temps_cfg.tempsSize > 0 || temps_cfg.lightPowerRatio >= 0.0){
  } else if(temps_cfg.fan_mode == PID || temps_cfg.fan_mode == LINEAR){  
    //float temperature = readDHTTemperature();
    if(!isnan(temperature)){
      n_times_temp_nan = 0;
      //if(temps_cfg.tempsSize > 0){
      if(temps_cfg.fan_mode == LINEAR){
        fan_speed = getLinearTempSpeed(temps_cfg, temperature);
        prev_cycle_was_PID = false;
      } else {
        // does PID 
        fan_speed = setPIDfanSpeed(temperature, temps_cfg.lightPowerRatio);
      }
    } else {
      if(n_times_temp_nan > 2){
        // when no temp profile default to the avg light power
        fan_speed = getAvgLightPower();
        prev_cycle_was_PID = false;
      } else {
        // to not overflow the var n_times_temp_nan, only increases til 3
        Serial.println("temp was NAN, will be using AvgLightPower after 3x!");
        n_times_temp_nan++;
        return;
      }
    }
    fan_speed = max(min(max_fan_speed,fan_speed), min_fan_speed);
    setMainFanPwm(fan_speed);
  } else if(temps_cfg.fan_mode == DRYING){
    processDryingMode();  
    prev_cycle_was_PID = false;  
  } else {
    prev_cycle_was_PID = false;  
  }
}

void setupFan(){
  linear_temp_offset = getMemLinearTempOffset();
  restoreFanMode();
  min_fan_speed = getFanMinSpeed();
  max_fan_speed = getFanMaxSpeed();
  if (max_fan_speed == 0)
    max_fan_speed = MAX_PWM_FAN;

  prev_main_fan_speed = getMemFanManualSpeed();

  perc_time_on = getMemFanDryPercTimeOn();
  fan_period_mins = getMemFanPeriodMins();

//  // Setting a timer to use in startAutoFan()
//  timer_fan = timerBegin(1, 8000, true);
//  timerAttachInterrupt(timer_fan, &startAutoFan, true);
  
  // makes sure the fans starts
  // the fan can have a speed as low as 40, but can not start with that
  setPwmVal(NLIGHTS, 160);
  delay(700);
  setMainFanPwm(prev_main_fan_speed);
}
