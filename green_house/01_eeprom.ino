// include library to read and write from flash memory
//#include <EEPROM.h>
#include <Preferences.h>

Preferences prefs;


// myPrefs.putBytes(“tag”, myArr, 30*sizeof(int))’.

// Remove all preferences under the opened namespace
//preferences.clear();
// Or remove the counter key only
//preferences.remove("counter");

// define the number of bytes you want to access for stage
//#define EEPROM_STAGE_SIZE 1
//#define EEPROM_IDX 0

// define the number of bytes you want to access for fan mode and temp
//#define EEPROM_T_SIZE 2
//#define EEPROM_T_IDX 1
//#define EEPROM_TM_IDX 2


// Stage
const char* sel_stage_key = "selstg";
int getMemStageVal(){
  //return EEPROM.read(EEPROM_IDX);
  return prefs.getUInt(sel_stage_key, 0);
}

void setMemStageVal(unsigned int stageVal){

  prefs.putUInt(sel_stage_key, stageVal);
  
  //EEPROM.write(EEPROM_IDX, stageVal);
  //EEPROM.commit();
}

const char* light_modes_key = "lightm";
void setMemLightModes(void * obj, int  size_obj){

  //prefs.putBytes(lights_key, vals, n_vals*sizeof(int));
  prefs.putBytes(light_modes_key, obj, size_obj);
  Serial.println("setStruct: size is "+String(size_obj));
}

void getMemLightModes(void * object){
  size_t schLen = prefs.getBytes(light_modes_key, NULL, NULL);
  
  Serial.println("getStruct: size is "+String(schLen));

  if(schLen == 0)
    return;

  char cbuffer[schLen]; // prepare a buffer for the data, why???

  prefs.getBytes(light_modes_key, cbuffer, schLen);
  memcpy(object, cbuffer, schLen);
}

// Fan temp
const char* fan_temp_key = "fant";
int getMemFanModeTemp(){
  //return EEPROM.read(EEPROM_T_IDX);
  return prefs.getInt(fan_temp_key, 0);
}

void setMemFanModeTemp(int temp){
//  EEPROM.write(EEPROM_T_IDX, temp);
//  EEPROM.commit();
  prefs.putInt(fan_temp_key, temp);
}

// Fan mode
const char* fan_mode_key = "fanm";
int getMemFanModeNum(){
  //return EEPROM.read(EEPROM_TM_IDX);
  return prefs.getInt(fan_mode_key, 0);
}

void setMemFanModeNum(int num){
//  EEPROM.write(EEPROM_TM_IDX, num);
//  EEPROM.commit();
  prefs.putInt(fan_mode_key, num);
}

void setupEEPROM(){
  // initialize EEPROM with predefined size
  // one byte for the sel stage num, another for the sel temp
  //EEPROM.begin(EEPROM_STAGE_SIZE+EEPROM_T_SIZE+n_stages*n_lights);

  // Open Preferences with my-app namespace. Each application module, library, etc
  // has to use a namespace name to prevent key name collisions. We will open storage in
  // RW-mode (second parameter has to be false, read only mode flag).
  // Note: Namespace name is limited to 15 chars.
  prefs.begin("greenkea", false);
}
