// include library to read and write from flash memory
#include <EEPROM.h>

// define the number of bytes you want to access
#define EEPROM_SIZE 1
#define EEPROM_IDX 0

int selectedStage = 0;


// TODO rm all NPWMS and fan stuff


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
    { // Stage 0 - sprout
    "sprout",
    // PWM for lights
    {0,0,63,63,0},
    // hour ON and OFF
    20,3
  },
  { // Stage 1 - grow
    "grow",
    // PWM for lights
    {0,0,150,150,30},
    // hour ON and OFF
    20,4
  },
  { // Stage 2 - late-grow
    "late_grow",
    // PWM for lights
    {120,120,255,255,127},
    // hour ON and OFF
    20,6
  },
  { // Stage 3 - flower
    "flower",
    // PWM for lights
    {255,255,255,255,255},
    // hour ON and OFF
    20,6
  },
  // Stage 4 - OFF
  stage_off,
  { // Stage 5
    "ON",
    // PWM for lights
    {255,255,255,255,255},
    // hour ON and OFF
    0,0
  }
};


void setStage(int stageVal){
  selectedStage = stageVal;
  EEPROM.write(EEPROM_IDX, stageVal);
  EEPROM.commit();
  updateStage();
}

void restoreStage(){
  // initialize EEPROM with predefined size
  EEPROM.begin(EEPROM_SIZE);
  selectedStage = EEPROM.read(EEPROM_IDX);
  
  if(selectedStage > NSTAGES-1 || selectedStage < 0)
    selectedStage = 0;
}

int getStage(){
  return selectedStage;
}

void processStageState(struct StageCfg stage, bool isStageON){
  int * pwmValsToSet;
  if(isStageON){
    pwmValsToSet = stage.pwmVals;
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
  int hour = getHour();
  bool isStageON = false;
  // Determining the state of the stage
  if (selStageCfg.n_hours_off == 0) {
    isStageON = true;
  } else if (selStageCfg.n_hours_off == 24) {
    isStageON = false;
  } else {
    // Checks if the selected stage is ON
    int hour_off = getHourOff(selStageCfg);
    int hour_on = selStageCfg.hour_on;
    //Serial.println("Updating stage: on "+String(hour_on)+"  off "+String(hour_off));
    if(hour_on <= hour_off){
      if(hour >= hour_on && hour < hour_off)
        isStageON = true;
    } else {
      if(hour >= hour_on || hour < hour_off)
        isStageON = true;
    }
  }
  // Precessing the stage's state
  if(isStageON != prev_isStageON || selectedStage != prev_selectedStage) {
    if(isStageON == false && selectedStage == prev_selectedStage){
    // when swiching the state off, saves the light vals
      pwmValsInfo pwmInfo = getPwmVals();  
      for(int i=0; i<NLIGHTS;i++){
        all_modes[selectedStage].pwmVals[i] = pwmInfo.vals[i];
      }
    }
    processStageState(selStageCfg, isStageON);
    Serial.printf("%ih: Stage updated %i->%i state %s->%s\n", 
      hour, prev_selectedStage, selectedStage, 
      prev_isStageON?"ON":"OFF", isStageON?"ON":"OFF"
    );
    prev_isStageON = isStageON;
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

  // Setting a timer to run updateStage()
  // prescaler 8000 - 10KHz base signal (base freq is 80 MHz) uint16 65,535
  timer = timerBegin(0, 8000, true);
  timerAttachInterrupt(timer, &timerHandler, true);
  // alarm to updateStage every  5 mins
  timerAlarmWrite(timer, 5*600000, true);
  timerAlarmEnable(timer);

  timerHandler();
}
