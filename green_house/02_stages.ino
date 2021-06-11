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
    8,
    // temps
    {0 ,20,24,27 ,30 ,33 ,36 ,40},
    // fanSpeeds
    {40,45,70,120,200,230,245,255}
};

TempCfg silent_temp_config = { 
      // FAN
    // tempsSize
    8,
    // temps
    {0 ,20,24,27 ,30 ,33 ,36 ,40},
    // fanSpeeds
    {40,45,60,100,130,180,220,255}
};


// Config  for all stages
StageCfg stage_off = { 
  // Stage OFF
  "OFF",
  // PWM
  {0,0,0,0,0,40},
  // hour ON and OFF
  0,0,
  {0,{},{}}
};

#define NSTAGES 7
StageCfg all_modes[NSTAGES] {
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
// have a max humid and temp, where if  above fan goes up, til back normal
void setFanSpeedFromStage(){
  StageCfg selStageCfg = all_modes[selectedStage];
  TempCfg temps_cfg = selStageCfg.temp_config;
  if(temps_cfg.tempsSize > 0){
    float temperature = readDHTTemperature();
    if(!isnan(temperature)){
      
      for(int i=0; i<temps_cfg.tempsSize;i++){
        
        if(temperature < temps_cfg.temps[i]){
            int pwmVal = 
                temps_cfg.fanSpeeds[i]-(temps_cfg.fanSpeeds[i]-temps_cfg.fanSpeeds[i-1])*
                ((temps_cfg.temps[i]-temperature)/(temps_cfg.temps[i]-temps_cfg.temps[i-1]));

            setPwmVal(NLIGHTS, pwmVal);
            //Serial.println( String(i)+" setting: fan to"+String(pwmVal)+" T"+String(temperature) );
            break;
          }
          // above max temp
          if(i == temps_cfg.tempsSize-1){
            setPwmVal(NLIGHTS, temps_cfg.fanSpeeds[i]);
          }
      }
    }else {
      setPwmVal(NLIGHTS, selStageCfg.pwmVals[NLIGHTS]);
      Serial.println("no temps to  set NAN");
    }
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
    processStage(selStageCfg, isStageON);
    Serial.printf("%ih: Stage updated %i->%i state %s->%s\n", 
      hour, prev_selectedStage, selectedStage, 
      prev_isStageON?"ON":"OFF", isStageON?"ON":"OFF"
    );
    prev_isStageON = isStageON;
    prev_selectedStage = selectedStage;
  }
  //setFanSpeedFromStage(selStageCfg);
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
  // alarm to updateStage every  3 mins
  timerAlarmWrite(timer, 5*600000, true);
  timerAlarmEnable(timer);
  
  timerHandler();
  
}
