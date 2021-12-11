// include library to read and write from flash memory
#include <Preferences.h>

Preferences prefs;


// Stage
const char* sel_stage_key = "selstg";
int getMemStageVal(){
  return prefs.getUInt(sel_stage_key, 0);
}

void setMemStageVal(unsigned int stageVal){
  prefs.putUInt(sel_stage_key, stageVal);
}

const char* light_modes_key = "lightm";
void setMemLightModes(void * obj, int  size_obj){

  //prefs.putBytes(lights_key, vals, n_vals*sizeof(int));
  prefs.putBytes(light_modes_key, obj, size_obj);
  //Serial.println("setStruct: size is "+String(size_obj));
}

void getMemLightModes(void * object){
  size_t schLen = prefs.getBytes(light_modes_key, NULL, NULL);
  //Serial.println("getStruct: size is "+String(schLen));

  if(schLen == 0)
    return;

  char cbuffer[schLen]; // prepare a buffer for the data, why???
  prefs.getBytes(light_modes_key, cbuffer, schLen);
  memcpy(object, cbuffer, schLen);
}

// Fan temp
const char* fan_temp_key = "fant";
int getMemFanModeTemp(){
  return prefs.getInt(fan_temp_key, 0);
}

void setMemFanModeTemp(int temp){
  prefs.putInt(fan_temp_key, temp);
}

// Fan mode
const char* fan_mode_key = "fanm";
int getMemFanModeNum(){
  return prefs.getInt(fan_mode_key, 0);
}

void setMemFanModeNum(int num){
  prefs.putInt(fan_mode_key, num);
}

void setupEEPROM(){
  // Open Preferences with my-app namespace. Each application module, library, etc
  // has to use a namespace name to prevent key name collisions. We will open storage in
  // RW-mode (second parameter has to be false, read only mode flag).
  // Note: Namespace name is limited to 15 chars.
  prefs.begin("greenkea", false);
}
