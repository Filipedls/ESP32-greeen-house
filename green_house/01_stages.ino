// include library to read and write from flash memory
#include <EEPROM.h>

// define the number of bytes you want to access
#define EEPROM_SIZE 1

struct StageCfg {
    int pwmVals[6];
    int hour_on;
    int hour_off;
};

// Config  for all stages
StageCfg all_modes[2] {
  { // Stage 1
    // PWM
    {0,1,2,3,4,5},
    // hour ON and OFF
    13,8
  },
  { // Stage 2
    // PWM
    {9,8,7,6,5,4},
    // hour ON and OFF
    9,19
  }
};


void updateStage(){
  int hour = getHour();
  Serial.println("Updating stage ("+String(hour)+"h)");
  
  for(int  sel_stage  = 0; sel_stage < 2;sel_stage++){
    StageCfg selStageCfg = all_modes[sel_stage];
    if(selStageCfg.hour_on <= selStageCfg.hour_off){
      if(hour >= selStageCfg.hour_on && hour < selStageCfg.hour_off){
        Serial.println("mode "+String(sel_stage)+" ON");
      }  else {
        Serial.println("mode "+String(sel_stage)+" OFF");
      }
    } else {
      if(hour >= selStageCfg.hour_on || hour < selStageCfg.hour_off){
        Serial.println("mode "+String(sel_stage)+" ON");
      }  else {
        Serial.println("mode "+String(sel_stage)+" OFF");
      }
    }
  }
}

int selectedStage = 0;
void setStage(int stageVal){
  selectedStage = stageVal;
  EEPROM.write(0, stageVal);
  EEPROM.commit();
  Serial.print("CB "+String(stageVal));
}

int getStage(){
  return selectedStage;
}

void stagesSetup(){
  // initialize EEPROM with predefined size
  EEPROM.begin(EEPROM_SIZE);

  selectedStage = EEPROM.read(0);
}
