// include library to read and write from flash memory
#include <EEPROM.h>

// define the number of bytes you want to access
#define EEPROM_SIZE 1
#define EEPROM_IDX 0

int selectedStage = 0;

struct StageCfg {
    char sname[20]; 
    int pwmVals[NPWMS];
    int hour_on;
    int hour_off;
};

// Config  for all stages
StageCfg stage_off = { 
  // Stage OFF
  "OFF",
  // PWM
  {0,0,0,0,0,40},
  // hour ON and OFF
  0,0
};

#define NSTAGES 4
StageCfg all_modes[NSTAGES] {
  { // Stage 1
    "stg1_12_20",
    // PWM
    {0,0,255,255,255,80},
    // hour ON and OFF
    12,20
  },
  { // Stage 2
    "stg2_16_23",
    // PWM
    {0,0,0,255,255,70},
    // hour ON and OFF
    16,23
  },
  // Stage 3 - OFF
  stage_off,
  { // Stage 4
    "ON",
    // PWM
    {255,255,255,255,255,123},
    // hour ON and OFF
    0,24
  }
};


void setStage(int stageVal){
  selectedStage = stageVal;
  EEPROM.write(EEPROM_IDX, stageVal);
  EEPROM.commit();
  Serial.print("M ");
  updateStage();
}

int getStage(){
  return selectedStage;
}

void processStage(struct StageCfg stage, bool isStageON){
  if(isStageON){
    setPwmVals(stage.pwmVals);
  } else {
    setPwmVals(stage_off.pwmVals);
  }
}

// AUTO temp humid  control
// have a max humid where if  above fan goes up, til back normal

int prev_isStageON = -1;
int prev_selectedStage = -1;
void updateStage(){
  int hour = getHour();
  //Serial.print("Updating stage "+String(selectedStage)+" ("+String(hour)+"h)... ");
  
  StageCfg selStageCfg = all_modes[selectedStage];
  bool isStageON = false;
  // Checks if the selected stage is ON
  if(selStageCfg.hour_on <= selStageCfg.hour_off){
    if(hour >= selStageCfg.hour_on && hour < selStageCfg.hour_off)
      isStageON = true;
  } else {
    if(hour >= selStageCfg.hour_on || hour < selStageCfg.hour_off)
      isStageON = true;
  }
  if(isStageON != prev_isStageON || selectedStage != prev_selectedStage) {
    processStage(selStageCfg, isStageON);
    Serial.printf("%ih: Stage updated %i->%i state %s->%s\n", 
      hour, prev_selectedStage, selectedStage, 
      prev_isStageON?"ON":"OFF", isStageON?"ON":"OFF"
    );
    prev_isStageON = isStageON;
    prev_selectedStage = selectedStage;
  }
}

// sensorPin event
//const int sensorPin = 27;
//void IRAM_ATTR attachHaddler() {
//  Serial.println("event sensor");
//}

// Semaphores for var access syncronization
// volatile int var_to_sync;
//portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
//portENTER_CRITICAL_ISR(&timerMux);
//var_to_sync++;
//portEXIT_CRITICAL_ISR(&timerMux);

hw_timer_t * timer = NULL;

void IRAM_ATTR timerHandler() {
  updateStage();
  //logTempHumidToGS();
}

void setupStages(){
  // initialize EEPROM with predefined size
  EEPROM.begin(EEPROM_SIZE);
  selectedStage = EEPROM.read(EEPROM_IDX);

  // attachInterrupt example
//  // button mode INPUT, INPUT_PULLUP: no need for 5v
//  pinMode(sensorPin, INPUT_PULLUP);
//  // CHANGE or RISING mode
//  attachInterrupt(digitalPinToInterrupt(sensorPin), attachHaddler, CHANGE);

  // Setting a timer to run updateStage()
  // prescaler 8000 - 10KHz base signal (base freq is 80 MHz) uint16 65,535
  timer = timerBegin(0, 8000, true);
  timerAttachInterrupt(timer, &timerHandler, true);
  // alarm to updateStage every  15 mins
  timerAlarmWrite(timer, 2*600000, true);
  timerAlarmEnable(timer);
  
  timerHandler();
  
}
