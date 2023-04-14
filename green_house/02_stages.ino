// TODO clean up pre-stage vars
// - state 2 (red mode) vars should be part of light stage - BUG: photo mode at 00:01 is red!
// - auto dim light, if bellow 20 is auto, eg 2 is 2C above set fan temp; 0 turn it off?

// TODO remove non obvious savelightmodes
int selectedStage = 0;

int min_light_val = 10;

// TODO move to pwm utils
#define MAX_PWM_LIGHTS 255

// todo change to uint8_t
// todo add secondary light cycles (red, uv, etc)
struct StageCfg {
    char sname[20]; 
    int pwmVals[NLIGHTS];
    // hour (24h) that the light goes on
    int hour_on;
    // n hours off
    int n_hours_off;
    bool dim_lights;
};

// Config  for all stages
StageCfg stage_off = { 
  // Stage OFF
  "OFF",
  // PWM for lights
  {0,0,0,0,0},
  // hour ON and OFF
  0,24,
  false
};

#define NSTAGES 6
StageCfg all_modes[NSTAGES] {
  { // Stage 1 - grow
    "Dim OFF",
    // PWM for lights
    {0,0,120,120,30},
    // hour ON and OFF
    20,4,
    false
  },
  { // Stage 2 - flower
    "Dim ON",
    // PWM for lights
    {255,255,255,255,255},
    // hour ON and OFF
    20,6,
    true
  },
  { // Stage 3 - open door
    "open door",
    // PWM for lights
    {20,20,255,255,15},
    // hour ON and OFF
    0,0,
    false
  },
  { // Stage 4 - photo
    "photo",
    // PWM for lights
    {0,0,255,255,0},
    // hour ON and OFF
    0,0,
    false
  },
  // Stage 5 - OFF
  stage_off,
  { // Stage 5
    "ON",
    // PWM for lights
    {255,255,255,255,255},
    // hour ON and OFF
    0,0,
    false
  }
};
// lights pwm config for state 2 of the light (20 mins bef and after lights on)
int state_s2_len_mins;// = 15;
int lights_s2_pwms[NLIGHTS] = {0};


// The temp at which we start to dim the lights
int start_dim_temp;// = 30;

/// Methods
int StateNum = 0;
void setPwmLight(int pwmID, int val){
  if(pwmID<NLIGHTS){
    setPwmLightVal(pwmID, val);
    // when light on, saves the new val in the selected stage
    if(StateNum==1)
      all_modes[selectedStage].pwmVals[pwmID] = val;
    //Serial.println("Light "+ String(pwmID)+" V" + String(val));
  }
}

void setPwmLightVal(int pwmID, int val){
  if(val < min_light_val && val != 0)
    val = min_light_val;
  setPwmVal(pwmID, val);
}

float dim_ratio = 1.0;
void setStage(int stageVal){
  int prev_sel_stage = selectedStage;
  selectedStage = stageVal;
  updateStage(NAN);
  if(selectedStage == NSTAGES-2)// off
    saveLightModes();
  
  if (selectedStage == 2)
    startOpenDoorTimer(prev_sel_stage);
  else
    setMemStageVal(stageVal);
    if(!all_modes[selectedStage].dim_lights)
      dim_ratio = 1.0;
}

void restoreLightVars(){
  selectedStage = getMemStageVal();
  
  getMemLightModes(all_modes, sizeof(all_modes));
  //Serial.println("restoreLightModes: Light modes restored! :) size is "+String(sizeof(all_modes)));

  max_temp_lights = getLightOffsTempMem();
  start_dim_temp = getStartDimTempMem();

  state_s2_len_mins = getMemState2LenMins();
  lights_s2_pwms[NLIGHTS-1] = getMemState2RedVal();
}

void saveLightModes(){
  setMemLightModes(all_modes, sizeof(all_modes));
  Serial.println("Light modes saved! :)");
}

int getStage(){
  return selectedStage;
}

void setLightsOffTemp(int temp){
  max_temp_lights = temp;
  setLightOffsTempMem(temp);
}

void setState2LenMins(int len_mins){
  if(len_mins >= 0 && len_mins <= 60){
    state_s2_len_mins = len_mins;
    setMemState2LenMins(len_mins);
  }
}

void setState2RedVal(int red_val){
  if(red_val >= 0 && red_val <= MAX_PWM_LIGHTS){
    lights_s2_pwms[NLIGHTS-1] = red_val;
    setMemState2RedVal(red_val);
  }
}

void processStageState(struct StageCfg stage, int StateNum){
  int * pwmValsToSet;
  if(StateNum==1){
    pwmValsToSet = stage.pwmVals;
  } else if(StateNum==2){
    pwmValsToSet = lights_s2_pwms;
  } else {
    pwmValsToSet = stage_off.pwmVals;
  }
  // only sets the lights, fan is independently controled
  for(int i=0; i<NLIGHTS;i++){
    setPwmLightVal(i, pwmValsToSet[i]);
  }
}


// hour_on  hour_off, n_hours_off //

void setHourOn(int hour_on){
  all_modes[selectedStage].hour_on = hour_on;
  updateStage(NAN);
  saveLightModes();
}
void setNHoursOff(int n_hours_off){
  all_modes[selectedStage].n_hours_off = n_hours_off;
  updateStage(NAN);
  saveLightModes();
}
int getHourOff(struct StageCfg stage){
  int hour_off = stage.hour_on - stage.n_hours_off;
  if (hour_off < 0)
    hour_off = hour_off + 24;
  return hour_off;
}

struct StageCfg getSelectedStage(){
  return all_modes[selectedStage];  
}

// TIMER stage open door: 
//   goes back to prev stage after 20mins
hw_timer_t * timer_open_door = NULL;
int timer_prev_sel_stage = -1;
void startOpenDoorTimer(int prev_stage){
  // (re)start alarm to startAutoFan in 20 mins
  timerAlarmDisable(timer_open_door);
  timerRestart(timer_open_door);
  timerAlarmWrite(timer_open_door, 20*600000, false);
  timerAlarmEnable(timer_open_door);
  
  timer_prev_sel_stage = prev_stage;
}

void IRAM_ATTR stopOpenDoorTimer(){
  if(timer_prev_sel_stage !=-1 && selectedStage == 2){
    selectedStage = timer_prev_sel_stage;
  }
  timer_prev_sel_stage = -1;
}

// Temperature based light dimming
// If the fan can't keep up with the lights, the temperature gets too high
// So if the temp goes above  'start_temp', we start dimming the lights

// the temp tolerance interval
float temp_tol = 0.5;
float tdim_kP = 0.03;

void checkTempToDimLights(struct StageCfg stage, float temperature){
  
  float temp_err = temperature - start_dim_temp;
  
  if(temperature > start_dim_temp && dim_ratio > 0.0){
    dim_ratio = max(float(0.0), dim_ratio-(tdim_kP*temp_err));
  } else if(temperature < (start_dim_temp-temp_tol) && dim_ratio < 1.0){
    dim_ratio = min(float(1.0), dim_ratio-(tdim_kP*(temp_err+temp_tol)) );
  } else {
    return;  
  }
  //Serial.println("diming lights > dim_ratio " + String(dim_ratio));
    
  // only sets the lights, fan is independently controled
  for(int i=0; i<NLIGHTS;i++){
    if(stage.pwmVals[i] > 0)
      setPwmLightVal(i, int(dim_ratio*stage.pwmVals[i]));
  }
}

void setStartDimTemp(int new_val){
  start_dim_temp = new_val;
  setStartDimTempMem(start_dim_temp);
}
// STAGE - Lights //

int prev_StateNum = -1;
int prev_selectedStage = -1;
void updateStage(float temperature){
  StageCfg selStageCfg = all_modes[selectedStage];
  //float temperature = readDHTTemperature();
  if(!isnan(temperature) && temperature > max_temp_lights){
    // turns off the lights by forcing the state to 0
    processStageState(selStageCfg, 0);
    prev_StateNum = 0;
    Serial.println("updateStage > TEMPS ABOVE " + String(max_temp_lights) +" (" +String(temperature) + ") LIGHTS OFF!");
    return; 
  }
  //int hour = getHour();
  int hour, mins;
  getHourMin(&hour, &mins);
  //Serial.println("Updating stage > h:"+String(hour)+" m:"+String(mins));
  StateNum = 0;
  // Determining the state of the stage
  if (selStageCfg.n_hours_off == 0) {
    StateNum = 1;
  } else if (selStageCfg.n_hours_off == 24) {
    StateNum = 0;
  } else {
    // Checks if the selected stage is ON
    int hour_off = getHourOff(selStageCfg);
    int hour_on = selStageCfg.hour_on;
    //Serial.println("Updating stage: on "+String(hour_on)+"  off "+String(hour_off));
    if(hour_on <= hour_off){
      if(hour >= hour_on && hour < hour_off)
        StateNum = 1;
    } else {
      if(hour >= hour_on || hour < hour_off)
        StateNum = 1;
    }
    // pre-state
    if((hour == hour_on && mins < state_s2_len_mins) || ((hour == hour_off && mins < state_s2_len_mins)))
      StateNum = 2;
  }
  // Precessing the stage's state
  if(StateNum != prev_StateNum || selectedStage != prev_selectedStage) {
//    if(StateNum != 1 && prev_StateNum == 1 && selectedStage == prev_selectedStage){
//    // when swiching the state off, saves the light vals
//      //saveLightModes();
//      for(int i=0; i<NLIGHTS;i++){
//        if(stage.pwmVals[i] != 0)
//          saveLightModes();
//          break;
//      }
//    }
    // when turning on resets the dim_ratio
    if(selStageCfg.dim_lights && StateNum == 1){
      dim_ratio = 1.0;
    }
    processStageState(selStageCfg, StateNum);
    Serial.printf("%ih: Stage updated %i->%i state %s->%s\n", 
      hour, prev_selectedStage, selectedStage, 
      prev_StateNum?"ON":"OFF", StateNum?"ON":"OFF"
    );
    prev_StateNum = StateNum;
    prev_selectedStage = selectedStage;
  } else if (!isnan(temperature) && selStageCfg.dim_lights && StateNum == 1){
    checkTempToDimLights(selStageCfg, temperature);
  }
}

/*
void sunRise(struct StageCfg selStageCfg){

    // 2 min
  int sunRiseLenght = 2*60000;
  // half the time for red rise
  int sunRedLenght = sunRiseLenght/2.0;
  int maxRedLightVal = selStageCfg.pwmVals[NLIGHTS-1];
  int initRedLightVal = 6;
  int minAdjustDelayRed = sunRedLenght / (maxRedLightVal -initRedLightVal);

  Serial.println("SunRise: starting red at "+String(minAdjustDelayRed)+" ms; red len: "+String(sunRedLenght)+" ms");
  
  for(int i=initRedLightVal; i<maxRedLightVal;i++){
    // setting red light pwm
    setPwmLightVal(NLIGHTS-1, i);
    Serial.print(String(i)+" ");
    delay(minAdjustDelayRed);
  }
  // computing the max light val
  int maxLightVal = 0;
  for(int i=0; i<NLIGHTS;i++){
    if(selStageCfg.pwmVals[i] > maxLightVal){
      maxLightVal = selStageCfg.pwmVals[i] ;
    }
  }
  
  int sunWhiteRiseLenght = sunRiseLenght - sunRedLenght;
  int initLightVal = 6;
  int minAdjustDelay = sunWhiteRiseLenght / (maxLightVal-initLightVal);
  
  Serial.println("SunRise: starting at "+String(minAdjustDelay)+" ms; len: "+String(sunRiseLenght)+" ms");

  for(int i=initLightVal; i<maxLightVal;i++){

    (i / float(maxLightVal))*;

    // only sets the lights, fan is independently controled
    for(int j=0; j<NLIGHTS-1;j++){

      int ithLightPwm = initLightVal + ( (i-initLightVal)*(selStageCfg.pwmVals[j] / float(maxLightVal) ) ) ;
      
      setPwmLightVal(j, ithLightPwm);
    }
    Serial.print(String(i)+" ");
    delay(minAdjustDelay);
  }
}

*/
// sensorPin event
//const int sensorPin = 27;
//void IRAM_ATTR attachHaddler() {
//  Serial.println("event sensor");
//}
//
// Semaphores for var access syncronization
// volatile int var_to_sync;
//portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
//portENTER_CRITICAL_ISR(&timerMux);
//var_to_sync++;
//portEXIT_CRITICAL_ISR(&timerMux);
//
// attachInterrupt example
//  // button mode INPUT, INPUT_PULLUP: no need for 5v
//  pinMode(sensorPin, INPUT_PULLUP);
//  // CHANGE or RISING mode
//  attachInterrupt(digitalPinToInterrupt(sensorPin), attachHaddler, CHANGE);


//hw_timer_t * timer = NULL;
//
//void IRAM_ATTR timerHandler() {
//  //if(time_failed)
//  updateStage();
//}

void setupStages(){
  restoreLightVars();

  // Setting a timer to use in startAutoFan()
  timer_open_door = timerBegin(1, 8000, true);
  timerAttachInterrupt(timer_open_door, &stopOpenDoorTimer, true);

//  // Setting a timer to run updateStage()
//  // prescaler 8000 - 10KHz base signal (base freq is 80 MHz) uint16 65,535
//  timer = timerBegin(0, 8000, true);
//  timerAttachInterrupt(timer, &timerHandler, true);
//  // alarm to updateStage every  5 mins
//  timerAlarmWrite(timer, 5*600000, true);
//  timerAlarmEnable(timer);

  updateStage(NAN);
}
