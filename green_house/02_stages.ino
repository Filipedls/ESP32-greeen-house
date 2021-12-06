// TODO agg pre-stage vars
// TODO sync updateStage() and updateFanSpeed() (runs after)

int selectedStage = 0;

// todo change to uint8_t
struct StageCfg {
    char sname[20]; 
    int pwmVals[NLIGHTS];
    // hour (24h) that the light goes on
    int hour_on;
    // n hours off
    //int hour_off;
    int n_hours_off;
};

// Config  for all stages
StageCfg stage_off = { 
  // Stage OFF
  "OFF",
  // PWM for lights
  {0,0,0,0,0},
  // hour ON and OFF
  0,24
};

#define NSTAGES 6
StageCfg all_modes[NSTAGES] {
  { // Stage 1 - grow
    "grow",
    // PWM for lights
    {0,0,150,150,30},
    // hour ON and OFF
    20,4
  },
  { // Stage 2 - flower
    "flower",
    // PWM for lights
    {255,255,255,255,255},
    // hour ON and OFF
    20,6
  },
  { // Stage 3 - late-grow
    "open door",
    // PWM for lights
    {20,20,255,255,90},
    // hour ON and OFF
    0,0
  },
  { // Stage 4 - sprout
    "photo",
    // PWM for lights
    {0,0,255,255,0},
    // hour ON and OFF
    0,0
  },
  // Stage 5 - OFF
  stage_off,
  { // Stage 5
    "ON",
    // PWM for lights
    {255,255,255,255,255},
    // hour ON and OFF
    0,0
  }
};


void setPwmLight(int pwmID, int val){
  if(pwmID<NLIGHTS){
    setPwmVal(pwmID, val);
    // saves the new val in the selected stage
    all_modes[selectedStage].pwmVals[pwmID] = val;
    Serial.println("Light "+ String(pwmID)+" V" + String(val));
  }
}

void setStage(int stageVal){
  selectedStage = stageVal;
  setMemStageVal(stageVal);
  updateStage();
}

void restoreStage(){
  selectedStage = getMemStageVal();
}

void saveStageLightsVals(){

//  int vals[NSTAGES*NLIGHTS];
//  for(int r=0; r < NSTAGES; r++){
//    for(int c=0; c < NLIGHTS; c++){
//      vals[r*NLIGHTS+c] = all_modes[r].pwmVals[c];//, vals[r*n_lights+c]);
//    }
//  }
//  setMemStageLightVals(NSTAGES*NLIGHTS, vals);


  setMemLightModes(all_modes, sizeof(all_modes));

  
  Serial.println("Light stage vals saved! :)");
}

void restoreStageLightsVals(){
//  int read_vals[NSTAGES*NLIGHTS];
//  int size_vals = getMemStageLightVals(NSTAGES*NLIGHTS, read_vals);
//  if(size_vals == NSTAGES*NLIGHTS) {
//    for(int r=0; r < NSTAGES; r++){
//      for(int c=0; c < NLIGHTS; c++){
//        
//        int read_val = read_vals[r*NLIGHTS+c];
//        if(read_val >= 0 && read_val <= 255)
//          all_modes[r].pwmVals[c] = read_val;
//      }
//    }
//    Serial.println("Light stage vals restored! :)");
//  } else {
//    Serial.println("Light stage vals not restored! :( size: "+String(size_vals));
//  }

  getMemLightModes(all_modes);
  Serial.println("Light stage vals restored! :)");
  
  Serial.println("restoreStageLightsVals: size is "+String(sizeof(all_modes)));
}

int getStage(){
  return selectedStage;
}

int state_mode_2_pwms[NLIGHTS] = {0,0,0,0,30};
void processStageState(struct StageCfg stage, int StageNum){
  int * pwmValsToSet;
  if(StageNum==1){
    pwmValsToSet = stage.pwmVals;
  } else if(StageNum==2){
    pwmValsToSet = state_mode_2_pwms;
  } else {
    pwmValsToSet = stage_off.pwmVals;
  }
  // only sets the lights, fan is independently controled
  for(int i=0; i<NLIGHTS;i++){
    setPwmVal(i, pwmValsToSet[i]);
  }
}


// hour_on  hour_off, n_hours_off //

void setHourOn(int hour_on){
  all_modes[selectedStage].hour_on = hour_on;
  updateStage();
}
void setNHoursOff(int n_hours_off){
  all_modes[selectedStage].n_hours_off = n_hours_off;
  updateStage();
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

// STAGE - Lights //

int prev_isStageON = -1;
int prev_selectedStage = -1;
void updateStage(){
  StageCfg selStageCfg = all_modes[selectedStage];
  //int hour = getHour();
  int hour, mins;
  getHourMin(&hour, &mins);
  //Serial.println("Updating stage > h:"+String(hour)+" m:"+String(mins));
  int StageNum = 0;
  // Determining the state of the stage
  if (selStageCfg.n_hours_off == 0) {
    StageNum = 1;
  } else if (selStageCfg.n_hours_off == 24) {
    StageNum = 0;
  } else {
    // Checks if the selected stage is ON
    int hour_off = getHourOff(selStageCfg);
    int hour_on = selStageCfg.hour_on;
    //Serial.println("Updating stage: on "+String(hour_on)+"  off "+String(hour_off));
    if(hour_on <= hour_off){
      if(hour >= hour_on && hour < hour_off)
        StageNum = 1;
    } else {
      if(hour >= hour_on || hour < hour_off)
        StageNum = 1;
    }
    // pre-stage
    if((hour == hour_on && mins < 20) || ((hour == hour_off && mins < 20)))
      StageNum = 2;
  }
  // Precessing the stage's state
  if(StageNum != prev_isStageON || selectedStage != prev_selectedStage) {
    if(StageNum != 1 && prev_isStageON == 1 && selectedStage == prev_selectedStage){
    /* TODO / when swiching the state off, saves the light vals
      pwmValsInfo pwmInfo = getPwmVals();  
      for(int i=0; i<NLIGHTS;i++){
        all_modes[selectedStage].pwmVals[i] = pwmInfo.vals[i];
      }
      /*/
      saveStageLightsVals();
    }
    processStageState(selStageCfg, StageNum);
    Serial.printf("%ih: Stage updated %i->%i state %s->%s\n", 
      hour, prev_selectedStage, selectedStage, 
      prev_isStageON?"ON":"OFF", StageNum?"ON":"OFF"
    );
    prev_isStageON = StageNum;
    prev_selectedStage = selectedStage;
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
    setPwmVal(NLIGHTS-1, i);
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
      
      setPwmVal(j, ithLightPwm);
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

hw_timer_t * timer = NULL;

void IRAM_ATTR timerHandler() {
  //if(time_failed)
  updateStage();
}

void setupStages(){
  restoreStage();
  restoreStageLightsVals();

  // Setting a timer to run updateStage()
  // prescaler 8000 - 10KHz base signal (base freq is 80 MHz) uint16 65,535
  timer = timerBegin(0, 8000, true);
  timerAttachInterrupt(timer, &timerHandler, true);
  // alarm to updateStage every  5 mins
  timerAlarmWrite(timer, 5*600000, true);
  timerAlarmEnable(timer);

  timerHandler();
}
