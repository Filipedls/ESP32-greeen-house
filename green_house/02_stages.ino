// TODO clean up pre-stage vars
// * call light start end > rise and set , from sun rise/set
// * sync fan mode with light stage, by having a int var in the StageCfg struct

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
    bool check_start_end;
};

// Config  for all stages
StageCfg stage_off = { 
  // Stage OFF
  "OFF",
  // PWM for lights
  {0,0,0,0,0},
  // hour ON and OFF
  0,24,
  false,
  false
};

#define NSTAGES 6
StageCfg all_modes[NSTAGES] {
  { // Stage 1 - grow
    "Seed",
    // PWM for lights
    {0,0,120,120,30},
    // hour ON and OFF
    20,4,
    false,
    true
  },
  { // Stage 2 - flower
    "Grow",
    // PWM for lights
    {255,255,255,255,150},
    // hour ON and OFF
    20,6,
    true,
    true
  },
  { // Stage 3 - open door
    "open door",
    // PWM for lights
    {20,20,255,255,15},
    // hour ON and OFF
    0,0,
    false,
    false
  },
  { // Stage 4 - photo
    "photo",
    // PWM for lights
    {0,0,255,255,0},
    // hour ON and OFF
    0,0,
    false,
    false
  },
  // Stage 5 - OFF
  stage_off,
  { // Stage 6
    "ON",
    // PWM for lights
    {255,255,255,255,255},
    // hour ON and OFF
    0,0,
    false,
    false
  }
};
// lights pwm config for state 2 of the light (20 mins bef and after lights on)
// int state_s2_len_mins;// = 15;
// int lights_s2_pwms[NLIGHTS] = {0};

// The temp at which we start to dim the lights
int start_dim_temp;// = 30;

int dim_priority_arr[NLIGHTS];

// both inclusive (light is on)
int lights_start_delay_mins_arr[NLIGHTS] = {0};// = {15,15,15,15,0};
//FILLARRAY(dim_priority_arr,1);
int lights_end_early_mins_arr[NLIGHTS]= {0};

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

  max_temp_lights = getLightOffsTempMem();
  start_dim_temp = getStartDimTempMem();

  // state_s2_len_mins = getMemState2LenMins();
  // lights_s2_pwms[NLIGHTS-1] = getMemState2RedVal();


  fillArray(dim_priority_arr, NLIGHTS, 1);
  getMemDimPrioArr(dim_priority_arr, sizeof(dim_priority_arr));

  getMemStartDelayArr(lights_start_delay_mins_arr, sizeof(lights_start_delay_mins_arr));

  getMemEndEarlyArr(lights_end_early_mins_arr, sizeof(lights_end_early_mins_arr));
  // TODORM
  // for(int i=0; i<NLIGHTS;i++){
  //   Serial.println(String(i)+" restoreLightVars > "+String(dim_priority_arr[i]));
  // }
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

  triggerTimer(timer_open_door, 20, false);
  Serial.println("startOpenDoorTimer > timer started");
  
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
float temp_tol = 0.3;
float tdim_kP = 0.015;

void setDimPrio(const int vals[]){
  for(int i=0; i<NLIGHTS;i++){
    dim_priority_arr[i] = vals[i];
  }
  setMemDimPrioArr(dim_priority_arr, sizeof(dim_priority_arr));
}

void switchLightStageDim(){
  if(all_modes[selectedStage].dim_lights == true){
    all_modes[selectedStage].dim_lights = false;
  } else {
    all_modes[selectedStage].dim_lights = true;
  }
}

bool getLightStageDim(){
  return all_modes[selectedStage].dim_lights;
}

int getStartDimTempInC(){
  if (start_dim_temp < min_fan_goal_temp){ 
    // automatic start_dim_temp
    return main_fan_goal_temp + start_dim_temp;
  } else {
    return start_dim_temp;
  }
}

bool checkTempToDimLights(int non_dim_light_vals[], float temperature, int dim_pwm_vals[]){
  // return: if pwm were updated or not

  int start_dim_temp_C = getStartDimTempInC(); // in celcious

  // Serial.print("checkTempToDimLights > non_dim_light_vals - ");
  // for(int i=0; i<NLIGHTS;i++) Serial.print(" "+String(non_dim_light_vals[i]));
  // Serial.println(" ");
  float temp_err = temperature - start_dim_temp_C;
  
  if(temperature > start_dim_temp_C && dim_ratio > 0.0){
    dim_ratio = max(float(0.0), dim_ratio-(tdim_kP*temp_err));
  } else if(temperature < (start_dim_temp_C-temp_tol) && dim_ratio < 1.0){
    dim_ratio = min(float(1.0), dim_ratio-(tdim_kP*(temp_err+temp_tol)) );
  } else if (dim_ratio == 1.0) {
    return false;  
  }

  float pri_2_sum_light = 0;
  float pri_0_sum_light = 0;
  for(int i=0; i<NLIGHTS;i++){
    if(dim_priority_arr[i] == 2){
      pri_2_sum_light += non_dim_light_vals[i];
      //pri_2_c++;
    } else if(dim_priority_arr[i] == 0){
      pri_0_sum_light += non_dim_light_vals[i];
      //pri_0_c++;
    } 
  }

  float dim_ratio_split = 0.5;
  if(pri_0_sum_light > 0 || pri_2_sum_light > 0)
    dim_ratio_split = pri_0_sum_light/(pri_0_sum_light+pri_2_sum_light);

  float map_dim_ratio_ab = max(double(0.0), (dim_ratio-dim_ratio_split)/(1.0-dim_ratio_split) );
  float map_dim_ratio_bl = min(float(1.0), dim_ratio/dim_ratio_split );

  //Serial.println("diming lights > map_dim_ratio_ab " + String(map_dim_ratio_ab)+" > map_dim_ratio_bl " + String(map_dim_ratio_bl));
  for(int i=0; i<NLIGHTS;i++){
    if(non_dim_light_vals[i] > 0){
      int light_val = 0;
      if(dim_priority_arr[i] == 1){
        light_val = int(dim_ratio*non_dim_light_vals[i]);
      } else if(dim_priority_arr[i] == 2){
        if (dim_ratio >= dim_ratio_split)
          light_val = int(map_dim_ratio_ab*non_dim_light_vals[i]);
        else
          light_val = 0;
      } else if(dim_priority_arr[i] == 0) {
        if (dim_ratio < dim_ratio_split )
          light_val = int(map_dim_ratio_bl*non_dim_light_vals[i]);
        else
          light_val = non_dim_light_vals[i];
      }
      
      dim_pwm_vals[i] = light_val;
    } else
      dim_pwm_vals[i] = 0;
  }
  return true; 
}

void setStartDimTemp(int new_val){
  start_dim_temp = new_val;
  setStartDimTempMem(start_dim_temp);
}


// PROCESS STAGE
// START END LIGHTS

void setStartDelayArr(const int vals[]){
  for(int i=0; i<NLIGHTS;i++){
    lights_start_delay_mins_arr[i] = vals[i];
  }
  setMemStartDelayArr(lights_start_delay_mins_arr, sizeof(lights_start_delay_mins_arr));
}

void setEndEarlyArr(const int vals[]){
  for(int i=0; i<NLIGHTS;i++){
    lights_end_early_mins_arr[i] = vals[i];
  }
  setMemEndEarlyArr(lights_end_early_mins_arr, sizeof(lights_end_early_mins_arr));
}

void switchLightStageCheckStartEnd(){
  if(all_modes[selectedStage].check_start_end == true){
    all_modes[selectedStage].check_start_end = false;
  } else {
    all_modes[selectedStage].check_start_end = true;
  }
}

bool getLightStageCheckStartEnd(){
  return all_modes[selectedStage].check_start_end;
}

void processStageState(struct StageCfg stage, int StateNum, int minsIn, float temperature, bool is_state_change){
  int * pwmValsToSet;
  int lights_start_end_pwm_vals[NLIGHTS];
  //Serial.println("processStageState > minsIn "+String(minsIn)+" stage.check_start_end "+String(stage.check_start_end));
  if(StateNum==1){

    if(stage.check_start_end){
      int minTillEnd = ((24-stage.n_hours_off)*60) - minsIn;
      //Serial.println("processStageState > minTillEnd "+String(minTillEnd));
      for(int i=0; i<NLIGHTS;i++){
        if(minsIn >= lights_start_delay_mins_arr[i] && minTillEnd >= lights_end_early_mins_arr[i])
          lights_start_end_pwm_vals[i] = stage.pwmVals[i];
        else
          lights_start_end_pwm_vals[i] = stage_off.pwmVals[i];
      }
      pwmValsToSet = lights_start_end_pwm_vals;
    } else{
      //Serial.println("processStageState > NOT check_start_end");
      pwmValsToSet = stage.pwmVals;
    }

  } else {
    pwmValsToSet = stage_off.pwmVals;
  }
  //Serial.print("processStageState > pwmValsToSet - ");
  //for(int i=0; i<NLIGHTS;i++) Serial.print(" "+String(pwmValsToSet[i]));
  //Serial.println(" ");

  int dim_pwm_vals[NLIGHTS];
  if(!isnan(temperature) && stage.dim_lights && StateNum == 1 && !is_state_change){
    bool dim_on = checkTempToDimLights(pwmValsToSet, temperature, dim_pwm_vals);
    //Serial.println("processStageState > dimming "+String(dim_on));
    if (dim_on)
      pwmValsToSet = dim_pwm_vals;
  } 

  //Serial.println("processStageState > setting pwm ");
  pwmValsInfo pwmInfo = getPwmVals();
  for(int i=0; i<NLIGHTS;i++){
    //Serial.print(String(pwmValsToSet[i])+" ");
    if(pwmValsToSet[i] != pwmInfo.vals[i])
      //Serial.println("processStageState > setting pwm "+String(i));
      setPwmLightVal(i, pwmValsToSet[i]);
  }
  //Serial.println(" ");
}


// STAGE - Lights //

int prev_StateNum = -1;
int prev_selectedStage = -1;
void updateStage(float temperature){
  StageCfg selStageCfg = all_modes[selectedStage];
  //float temperature = readDHTTemperature();
  if(!isnan(temperature) && temperature > max_temp_lights){
    // turns off the lights by forcing the state to 0
    processStageState(selStageCfg, 0, 0, NAN, true);
    prev_StateNum = 0;
    Serial.println("updateStage > TEMPS ABOVE " + String(max_temp_lights) +" (" +String(temperature) + ") LIGHTS OFF!");
    return; 
  }
  //int hour = getHour();
  int hour, mins;
  getHourMin(&hour, &mins);
  //Serial.println("Updating stage > h:"+String(hour)+" m:"+String(mins));
  StateNum = 0;


  int minsOn = 0;
  // Determining the state of the stage
  // if (selStageCfg.n_hours_off == 0) {
  //   StateNum = 1;
  // } else 
  if (selStageCfg.n_hours_off == 24) {
    StateNum = 0;
  } else {
    // Checks if the selected stage is ON
    int hour_off = getHourOff(selStageCfg);
    int hour_on = selStageCfg.hour_on;
    //Serial.println("Updating stage: on "+String(hour_on)+"  off "+String(hour_off));
    if(hour_on < hour_off){
      if(hour >= hour_on && hour < hour_off){
        StateNum = 1;
        minsOn = (hour-hour_on)*60+mins;
      }
    } else {
      if(hour >= hour_on){
        StateNum = 1;
        minsOn = (hour-hour_on)*60+mins;
      } else if (hour < hour_off){
        StateNum = 1;
        minsOn = (24-hour_on+hour)*60+mins;
      }
    }
    // // pre-state
    // if((hour == hour_on && mins < state_s2_len_mins) || ((hour == hour_off && mins < state_s2_len_mins)))
    //   StateNum = 2;
  }
  //Serial.println("updateStage > h:"+String(hour)+" m:"+String(mins)+" StateNum: "+String(StateNum)+" temp: "+String(temperature));
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

    processStageState(selStageCfg, StateNum, minsOn, temperature, true);
    Serial.printf("%ih: Stage updated %i->%i state %s->%s\n", 
      hour, prev_selectedStage, selectedStage, 
      prev_StateNum?"ON":"OFF", StateNum?"ON":"OFF"
    );
    prev_StateNum = StateNum;
    prev_selectedStage = selectedStage;
  } else if( StateNum == 1 && (selStageCfg.dim_lights || selStageCfg.check_start_end))
  // if (!isnan(temperature) && selStageCfg.dim_lights && StateNum == 1){
  //   checkTempToDimLights(selStageCfg.pwmVals, temperature);
  // }
     processStageState(selStageCfg, StateNum, minsOn, temperature, false);
}


void setupStages(){
  restoreLightVars();

  // Setting a timer to use in startAutoFan()
  // timer_open_door = timerBegin(1, 8000, true);
  // timerAttachInterrupt(timer_open_door, &stopOpenDoorTimer, true);

  timer_open_door = setupTimer(1, &stopOpenDoorTimer);

//  // Setting a timer to run updateStage()
//  // prescaler 8000 - 10KHz base signal (base freq is 80 MHz) uint16 65,535
//  timer = timerBegin(0, 8000, true);
//  timerAttachInterrupt(timer, &timerHandler, true);
//  // alarm to updateStage every  5 mins
//  timerAlarmWrite(timer, 5*600000, true);
//  timerAlarmEnable(timer);

  updateStage(NAN);
}
