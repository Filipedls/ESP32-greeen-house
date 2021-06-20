// include library to read and write from flash memory
#include <EEPROM.h>

// define the number of bytes you want to access
#define EEPROM_SIZE 1
#define EEPROM_IDX 0

int selectedStage = 0;

#define MAX_TEMPS_SIZE 10
struct TempCfg {
    int tempsSize;
    int temps[MAX_TEMPS_SIZE];
    int fanSpeeds[MAX_TEMPS_SIZE];
    float lightPowerRatio;
};

struct StageCfg {
    char sname[20]; 
    int pwmVals[NPWMS];
    int hour_on;
    int hour_off;
    TempCfg temp_config;
};


// temp config  for all stages
TempCfg main_temp_config = { 
      // FAN
    // tempsSize
    4,
    // temps
    {24,26 ,29 ,33 },
    // fanSpeeds
    {40,120,200,255},
    // lightPowerRatioWeight
    0.4
};

TempCfg silent_temp_config = { 
      // FAN
    // tempsSize
    4,
    // temps
    {24,26 ,30 ,33 },
    // fanSpeeds
    {40,90,170,255},
    // lightPowerRatioWeight
    0.3
};


// Config  for all stages
StageCfg stage_off = { 
  // Stage OFF
  "OFF",
  // PWM
  {0,0,0,0,0,0},
  // hour ON and OFF
  0,0,
  {0,{},{},0.0}
};

#define NSTAGES 8
StageCfg all_modes[NSTAGES] {
    { // Stage 0 - sprout
    "sprout_20_14",
    // PWM
    {0,0,63,63,0,50},
    // hour ON and OFF
    20,14,
    main_temp_config
  },
  { // Stage 1 - grow
    "grow_20_14",
    // PWM
    {0,0,127,127,63,90},
    // hour ON and OFF
    20,14,
    main_temp_config
  },
  { // Stage 2 - late-grow
    "late_grow_20_14",
    // PWM
    {0,0,255,255,127,140},
    // hour ON and OFF
    20,14,
    main_temp_config
  },
  { // Stage 3 - flower
    "flower_20_14",
    // PWM
    {255,255,255,255,255,160},
    // hour ON and OFF
    20,14,
    main_temp_config
  },
  { // Stage 4
    "stg_12_23",
    // PWM
    {0,0,255,255,180,100},
    // hour ON and OFF
    12,23,
    main_temp_config
  },
  { // Stage 5
    "stg_12_23_S",
    // PWM
    {0,0,0,255,80,70},
    // hour ON and OFF
    12,23,
    silent_temp_config
  },
  // Stage 6 - OFF
  stage_off,
  { // Stage 7
    "ON",
    // PWM
    {255,255,255,255,255,160},
    // hour ON and OFF
    0,24,
    main_temp_config
  }
};


void setStage(int stageVal){
  selectedStage = stageVal;
  EEPROM.write(EEPROM_IDX, stageVal);
  EEPROM.commit();
  Serial.print("M ");
  updateStage();
  setFanSpeedFromStage();
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


hw_timer_t * timer_fan = NULL;
bool timer_is_ON = false;
void stopAutoFan(){
  // (re)start alarm to startAutoFan in 30 mins
  timerAlarmWrite(timer_fan, 5*600000, false);
  if(!timer_is_ON){
    Serial.println("fan timer going ON");
    timerAlarmEnable(timer_fan);
    timer_is_ON = true;
  } else {
    Serial.println("fan timer going RESTART");
  }
}

void IRAM_ATTR startAutoFan(){
  if(timer_is_ON){
    Serial.println("fan timer going OFF");
    timerAlarmDisable(timer_fan);
    timer_is_ON = false;
  } else {
    Serial.println("fan timer going OFF, again?!");
  }
}


// AUTO temp humid  control
// have a max humid and temp, where if  above fan goes up, til back normal
void processTempCfg(struct TempCfg temps_cfg, float temperature){
  int pwmVal;
  // bellow min temp
  if(temperature < temps_cfg.temps[0]){
    pwmVal = temps_cfg.fanSpeeds[0];
  // above max temp
  } else if(temperature >= temps_cfg.temps[temps_cfg.tempsSize-1]) {
    pwmVal = temps_cfg.fanSpeeds[temps_cfg.tempsSize-1];
  // temp is somewheree in thee midle
  } else {
    for(int i=1; i<temps_cfg.tempsSize;i++){
    
      if(temperature < temps_cfg.temps[i]){
        pwmVal = temps_cfg.fanSpeeds[i]-(temps_cfg.fanSpeeds[i]-temps_cfg.fanSpeeds[i-1])*
            ((temps_cfg.temps[i]-temperature)/(temps_cfg.temps[i]-temps_cfg.temps[i-1]));
        //Serial.println( String(i)+" setting: fan to"+String(pwmVal)+" T"+String(temperature) );
        break;
      }
    }
  }
  // power ratio
  if(temps_cfg.lightPowerRatio > 0.0 && temps_cfg.lightPowerRatio <= 1.0){
    pwmValsInfo pwmInfo = getPwmVals();
  
    int lightPowerAvg = 0;
    for(int i=0; i<NLIGHTS;i++){
      lightPowerAvg += pwmInfo.vals[i];
    }
    lightPowerAvg = lightPowerAvg/NLIGHTS;
    //Serial.println("fan to"+String(pwmVal)+" T"+String(temperature) + " avg: "+ String(lightPowerAvg)  );
  
    pwmVal = temps_cfg.lightPowerRatio*(lightPowerAvg/255.0)*pwmVal + (1-temps_cfg.lightPowerRatio)*pwmVal;
  
    pwmVal = max(pwmVal, temps_cfg.fanSpeeds[0]);
    //Serial.println( "PWM fan "+String(pwmVal));
  }
  
  setMainFanPwm(pwmVal);
}


int n_times_temp_nan = 0;
void setFanSpeedFromStage(){
  if(timer_is_ON){
    Serial.println("temp timer_is_ON! nothing done");
    return;
  }
  StageCfg selStageCfg = all_modes[selectedStage];
  TempCfg temps_cfg = selStageCfg.temp_config;
  if(temps_cfg.tempsSize > 0){
    float temperature = readDHTTemperature();
    if(!isnan(temperature)){
      n_times_temp_nan = 0;
      processTempCfg(temps_cfg, temperature);
    }else {
      if(n_times_temp_nan > 2){ 
        setMainFanPwm(selStageCfg.pwmVals[NLIGHTS]);
        if(n_times_temp_nan == 3) {
          Serial.println("temp was NAN  3x! using  default from now on");
          n_times_temp_nan++;
        }
      } else {
        n_times_temp_nan++;
      }
    }
  } else {
    setMainFanPwm(selStageCfg.pwmVals[NLIGHTS]);
    //Serial.println("no temp config to set, using stage default");
  }
}

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
    processStageState(selStageCfg, isStageON);
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
  // initialize EEPROM with predefined size
  EEPROM.begin(EEPROM_SIZE);
  selectedStage = EEPROM.read(EEPROM_IDX);

  // Setting a timer to run updateStage()
  // prescaler 8000 - 10KHz base signal (base freq is 80 MHz) uint16 65,535
  timer = timerBegin(0, 8000, true);
  timerAttachInterrupt(timer, &timerHandler, true);
  // alarm to updateStage every  5 mins
  timerAlarmWrite(timer, 5*600000, true);
  timerAlarmEnable(timer);

  // Setting a timer to use in startAutoFan()
  timer_fan = timerBegin(1, 8000, true);
  timerAttachInterrupt(timer_fan, &startAutoFan, true);
  
  timerHandler();
  // makes sure the fans starts
  // the fan can have a speed as low as 40, but can not start with that
  setPwmVal(NLIGHTS, 80);
  delay(3000);
  setPwmVal(NLIGHTS, 40);
}
