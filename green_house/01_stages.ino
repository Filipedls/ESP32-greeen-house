// include library to read and write from flash memory
#include <EEPROM.h>

// define the number of bytes you want to access
#define EEPROM_SIZE 1
#define EEPROM_IDX 0

int selectedStage = 0;

const int pwmValsOFF[NPWMS] = {0,0,0,0,0,0};


const int sensorPin = 27;

struct StageCfg {
    char sname[20]; 
    int pwmVals[NPWMS];
    int hour_on;
    int hour_off;
};

// Config  for all stages
StageCfg all_modes[2] {
  { // Stage 1
    "stg1",
    // PWM
    {0,1,2,3,4,5},
    // hour ON and OFF
    13,15
  },
  { // Stage 2
    "stg2",
    // PWM
    {9,8,7,6,5,4},
    // hour ON and OFF
    9,23
  }
};


void setStage(int stageVal){
  selectedStage = stageVal;
  EEPROM.write(EEPROM_IDX, stageVal);
  EEPROM.commit();
  updateStage();
  Serial.println("now in stage "+String(stageVal));
}

int getStage(){
  return selectedStage;
}

void processStage(struct StageCfg stage, bool isStageON){
  if(isStageON){
    setPwmVals(stage.pwmVals);
  } else {
    setPwmVals(pwmValsOFF);
  }
}


void updateStage(){
  int hour = getHour();
  Serial.print("Updating stage "+String(selectedStage)+" ("+String(hour)+"h)... ");
  
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
  
  processStage(selStageCfg, isStageON);
  Serial.println("state "+String(isStageON?"ON":"OFF"));
}


void IRAM_ATTR attachHaddler() {
  Serial.println("event sensor");
}


void setupStages(){
  // initialize EEPROM with predefined size
  EEPROM.begin(EEPROM_SIZE);
  selectedStage = EEPROM.read(EEPROM_IDX);

  // attachInterrupt example
  // button mode INPUT
  pinMode(sensorPin, INPUT_PULLUP);
  // CHANGE or RISING mode
  attachInterrupt(digitalPinToInterrupt(sensorPin), attachHaddler, CHANGE);

  
}
