// TODO
//   * generalize get, sets (class for ints and strings? with get, set, key, etc)
//   * grow and connect config arrays

// include library to read and write from flash memory
#include <Preferences.h>

Preferences prefs;

// AUX
void setArray(void * obj, int  size_obj, const char* key){
  // e.g.: 
  // const char* key = "key1";
  // int array[SIZE] = {1};
  // setArray(array, sizeof(array), key);

  //prefs.putBytes(lights_key, vals, n_vals*sizeof(int));
  prefs.putBytes(key, obj, size_obj);
  //Serial.println("setStruct: size is "+String(size_obj));
}
void getArray(void * object, size_t sizeof_object, const char* key){
  // e.g.: 
  // const char* key = "key1";
  // int array[SIZE] = {1};
  // getArray(array, sizeof(array), key);

  size_t schLen = prefs.getBytes(key, NULL, NULL);
  //Serial.println("getStruct: size is "+String(schLen)+" "+String(sizeof_object));
  if(schLen == 0 || schLen != sizeof_object)
    return;
  
    //Serial.println("getArray > "+String(schLen) );
  char cbuffer[schLen]; // prepare a buffer for the data
  prefs.getBytes(key, cbuffer, schLen);
  memcpy(object, cbuffer, schLen);
}

// ==> LIGHTS
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
  // // prefs.putBytes(lights_key, vals, n_vals*sizeof(int));
  //prefs.putBytes(light_modes_key, obj, size_obj);
  //Serial.println("setStruct: size is "+String(size_obj));
  
  setArray(obj, size_obj, light_modes_key);
}
void getMemLightModes(void * object, size_t sizeof_object){
  // size_t schLen = prefs.getBytes(light_modes_key, NULL, NULL);
  // //Serial.println("getStruct: size is "+String(schLen)+" "+String(sizeof_object));
  // if(schLen == 0 || schLen != sizeof_object)
  //   return;
    
  // char cbuffer[schLen]; // prepare a buffer for the data
  // prefs.getBytes(light_modes_key, cbuffer, schLen);
  // memcpy(object, cbuffer, schLen);

  getArray(object, sizeof_object, light_modes_key);
}

// Light off temp
const char* lights_off_temp_key = "lightofft";
int getLightOffsTempMem(){
  return prefs.getInt(lights_off_temp_key, 34);
}
void setLightOffsTempMem(int temp){
  prefs.putInt(lights_off_temp_key, temp);
}

// // State2RedVal
// const char* state_s2_redv_key = "lights2rv";
// int getMemState2RedVal(){
//   return prefs.getInt(state_s2_redv_key, 20);
// }
// void setMemState2RedVal(int val){
//   prefs.putInt(state_s2_redv_key, val);
// }

// // state_2_len_mins
// const char* state_2_len_mins_key = "lights2lm";
// int getMemState2LenMins(){
//   return prefs.getInt(state_2_len_mins_key, 15);
// }
// void setMemState2LenMins(int len_mins){
//   prefs.putInt(state_2_len_mins_key, len_mins);
// }

// Dim priority
const char* dim_prio_key = "dimp";
void setMemDimPrioArr(void * obj, int  size_obj){
  // //prefs.putBytes(lights_key, vals, n_vals*sizeof(int));
  //prefs.putBytes(dim_prio_key, obj, size_obj);
  //Serial.println("setStruct: size is "+String(size_obj));

  setArray(obj, size_obj, dim_prio_key);
}
void getMemDimPrioArr(void * object, size_t sizeof_object){
  getArray(object, sizeof_object, dim_prio_key);
}

// Start Delay / End Early
const char* start_del_key = "lstartdel";
void setMemStartDelayArr(void * obj, int  size_obj){
  // //prefs.putBytes(lights_key, vals, n_vals*sizeof(int));
  //prefs.putBytes(dim_prio_key, obj, size_obj);
  //Serial.println("setStruct: size is "+String(size_obj));

  setArray(obj, size_obj, start_del_key);
}
void getMemStartDelayArr(void * object, size_t sizeof_object){
  getArray(object, sizeof_object, start_del_key);
}

const char* end_early_key = "lendear";
void setMemEndEarlyArr(void * obj, int  size_obj){
  // //prefs.putBytes(lights_key, vals, n_vals*sizeof(int));
  //prefs.putBytes(dim_prio_key, obj, size_obj);
  //Serial.println("setStruct: size is "+String(size_obj));

  setArray(obj, size_obj, end_early_key);
}
void getMemEndEarlyArr(void * object, size_t sizeof_object){
  getArray(object, sizeof_object, end_early_key);
}

// ==> FAN
// Fan temp
const char* fan_temp_key = "fant";
int getMemFanModeTemp(){
  return prefs.getInt(fan_temp_key, 27);
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

// Fan min speed
const char* fan_min_speed_key = "fanminspd";
int getFanMinSpeed(){
  return prefs.getInt(fan_min_speed_key, 45);
}
void setFanMinSpeed(int num){
  prefs.putInt(fan_min_speed_key, num);
}

// Fan max speed
const char* fan_max_speed_key = "fanmaxspd";
int getMemFanMaxSpeed(){
  return prefs.getInt(fan_max_speed_key, 0);
}
void setMemFanMaxSpeed(int num){
  prefs.putInt(fan_max_speed_key, num);
}

// Fan manual speed
const char* fan_manual_speed_key = "fanmanspd";
int getMemFanManualSpeed(){
  return prefs.getInt(fan_manual_speed_key, 60);
}
void setMemFanManualSpeed(int num){
  prefs.putInt(fan_manual_speed_key, num);
}

// Fan drying mode perc_time_on
const char* fan_perc_time_on_key = "fanptonm";
int getMemFanDryTimeOnMins(){
  return prefs.getInt(fan_perc_time_on_key, 5);
}
void setMemFanDryTimeOnMins(int num){
  prefs.putInt(fan_perc_time_on_key, num);
}

// Fan drying mode fan_period_mins
const char* fan_period_mins_key = "fandrypmin";
int getMemFanPeriodMins(){
  return prefs.getInt(fan_period_mins_key, 10);
}
void setMemFanPeriodMins(int num){
  prefs.putInt(fan_period_mins_key, num);
}

// Fan drying mode max_humidity
const char* fan_dry_humd_key = "fandryhumd";
int getFanDryMaxHumid(){
  return prefs.getInt(fan_dry_humd_key, 70);
}
void setMemFanDryMaxHumid(int num){
  prefs.putInt(fan_dry_humd_key, num);
}

// Fan linear_temp_offset
const char* linear_temp_offset_key = "fanltoset";
int getMemLinearTempOffset(){
  return prefs.getInt(linear_temp_offset_key, 3);
}
void setMemLinearTempOffset(int num){
  prefs.putInt(linear_temp_offset_key, num);
}

// tdim_start_temp
const char* start_dim_temp_key = "startdimtemp";
int getStartDimTempMem(){
  return prefs.getInt(start_dim_temp_key, 30);
}
void setStartDimTempMem(int num){
  prefs.putInt(start_dim_temp_key, num);
}

// mins silent
const char* mins_silent_key = "fanminsil";
int getMemMinsSilent(){
  return prefs.getInt(mins_silent_key, 5);
}
void setMemMinsSilent(int num){
  prefs.putInt(mins_silent_key, num);
}
// silent_fan_speed
const char* silent_fan_speed_key = "silfanspeed";
int getMemSilFanSpeed(){
  return prefs.getInt(silent_fan_speed_key, 120);
}
void setMemSilFanSpeed(int num){
  prefs.putInt(silent_fan_speed_key, num);
}

// ==> WiFi
// WiFi SSID
const char* wifissid_key = "wifissid";
String getWiFiSSID(){
  return prefs.getString(wifissid_key, "The Internet");
}
void setWiFiSSID(String val){
  prefs.putString(wifissid_key, val);
}


// WiFi Pass
const char* wifipass_key = "wifipass";
String getWiFiPass(){
  return prefs.getString(wifipass_key, "thepass");
}
void setWiFiPass(String val){
  prefs.putString(wifipass_key, val);
}

// WiFi IP
const char* wifiip_key = "wifiip";
String getWiFiIP(){
  return prefs.getString(wifiip_key, "192.168.1.231");
}
void setWiFiIP(String val){
  prefs.putString(wifiip_key, val);
}
// WiFi gateway
const char* wifigateway_key = "wifigw";
String getWiFigateway(){
  return prefs.getString(wifigateway_key, "192.168.1.1");
}
void setWiFigateway(String val){
  prefs.putString(wifigateway_key, val);
}

// ==> Other Configs (not related with growing)
// Log to the cloud
const char* logcloudflag_key = "logcloudflag";
int getLogCloudFlag(){
  return prefs.getInt(logcloudflag_key, 0);
}
void setMemLogCloudFlag(int val){
  prefs.putInt(logcloudflag_key, val);
}

// Log Server URL
const char* logserverurl_key = "logurl";
String getLogServerURL(){
  return prefs.getString(logserverurl_key, "http://logstuff.com");
}
void setLogServerURL(String val){
  prefs.putString(logserverurl_key, val);
}

// Log Server API key
const char* logServerAPIkey_key = "logkey";
String getLogServerAPIkey(){
  return prefs.getString(logServerAPIkey_key, "yekIPA");
}
void setLogServerAPIkey(String val){
  prefs.putString(logServerAPIkey_key, val);
}

// Log period
const char* log_period_mins_key = "logperiod";
int getLogPeriodMins(){
  return prefs.getInt(log_period_mins_key, 20);
}
void setMemLogPeriodMins(int val){
  prefs.putInt(log_period_mins_key, val);
}

// UTILS

void clearPrefs(){
  prefs.clear();
}

// remove
bool clearKey(String key){
  return prefs.remove(key.c_str());
}

void checkBootButtonClearPrefs(){
  //delay(100);
  // if boot button is pressed after the setup delay() call, the preferences will be cleared. Restart again for it to have effect.
  int boot_button_reading = digitalRead(0); // BOOT button's pin number is 0
  if (boot_button_reading == LOW) { // default state is HIGH
    Serial.println("BOOT button is pressed on setup, clearing preferences and restarting!");
    clearPrefs();
    delay(1000);
    ESP.restart();
  }
}

void setupEEPROM(){
  // Open Preferences with my-app namespace. Each application module, library, etc
  // has to use a namespace name to prevent key name collisions. We will open storage in
  // RW-mode (second parameter has to be false, read only mode flag).
  // Note: Namespace name is limited to 15 chars.
  prefs.begin("greenkea", false);
  // boot button as input to clear preferences
  pinMode(0, INPUT);
  checkBootButtonClearPrefs();
}
