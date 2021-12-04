// include library to read and write from flash memory
#include <EEPROM.h>

// define the number of bytes you want to access for stage
#define EEPROM_STAGE_SIZE 1
#define EEPROM_IDX 0

// define the number of bytes you want to access for fan mode and temp
#define EEPROM_T_SIZE 2
#define EEPROM_T_IDX 1
#define EEPROM_TM_IDX 2


// Stage
int getMemStageVal(){
  return EEPROM.read(EEPROM_IDX);
}

void setMemStageVal(int stageVal){
  EEPROM.write(EEPROM_IDX, stageVal);
  EEPROM.commit();
}

const int idx_lights_start = EEPROM_STAGE_SIZE+EEPROM_T_SIZE;
void setMemStageLightVals(int n_vals, int * vals){
  for(int rc_idx=0; rc_idx < n_vals; rc_idx++){
    EEPROM.write(idx_lights_start+rc_idx, vals[rc_idx]);
  }
  EEPROM.commit();
}

int getMemStageLightVal(int rc_idx){
  return EEPROM.read(idx_lights_start+rc_idx);
}

// Fan Mode
int getMemFanModeTemp(){
  return EEPROM.read(EEPROM_T_IDX);
}

void setMemFanModeTemp(int temp){
  EEPROM.write(EEPROM_T_IDX, temp);
  EEPROM.commit();
}
// Fan temp
int getMemFanModeNum(){
  return EEPROM.read(EEPROM_TM_IDX);
}

void setMemFanModeNum(int num){
  EEPROM.write(EEPROM_TM_IDX, num);
  EEPROM.commit();
}

void setupEEPROM(int n_stages, int n_lights){
  // initialize EEPROM with predefined size
  // one byte for the sel stage num, another for the sel temp
  EEPROM.begin(EEPROM_STAGE_SIZE+EEPROM_T_SIZE+n_stages*n_lights);
}
