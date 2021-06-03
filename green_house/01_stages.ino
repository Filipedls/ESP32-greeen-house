// include library to read and write from flash memory
#include <EEPROM.h>

// define the number of bytes you want to access
#define EEPROM_SIZE 1
#define EEPROM_IDX 0

int selectedStage = 0;
#define MAX_TEMPS_SIZE 10
struct StageCfg {
    char sname[20]; 
    int pwmVals[NPWMS];
    int hour_on;
    int hour_off;
    int tempsSize;
    int tempsVals[MAX_TEMPS_SIZE];
    int fansVals[MAX_TEMPS_SIZE];
};

// Config  for all stages
StageCfg stage_off = { 
  // Stage OFF
  "OFF",
  // PWM
  {0,0,0,0,0,40},
  // hour ON and OFF
  0,0,
  // FAN
  // tempsSize
  0,
  // tempsVals
  {},
  // fansVals
  {}
};

#define NSTAGES 5
StageCfg all_modes[NSTAGES] {
  { // Stage 1
    "stg1_12_20",
    // PWM
    {0,0,255,255,255,80},
    // hour ON and OFF
    12,20,
    // FAN
    // tempsSize
    7,
    // tempsVals
    {20,24,27,30,33,36,40},
    // fansVals
    {45,70,100,140,200,220,255}
  },
  { // Stage 2
    "stg2_16_23",
    // PWM
    {0,0,0,255,255,70},
    // hour ON and OFF
    16,23,
    // FAN
    // tempsSize
    7,
    // tempsVals
    {20,24,27,30,33,36,40},
    // fansVals
    {45,70,100,140,200,220,255}
  },
  { // Stage 3
    "stg3_20_14",
    // PWM
    {0,0,255,255,255,120},
    // hour ON and OFF
    20,14,
    // FAN
    // tempsSize
    7,
    // tempsVals
    {20,24,27,30,33,36,40},
    // fansVals
    {45,70,100,140,200,220,255}
  },
  // Stage 3 - OFF
  stage_off,
  { // Stage 4
    "ON",
    // PWM
    {255,255,255,255,255,123},
    // hour ON and OFF
    0,24,
    // FAN
    // tempsSize
    7,
    // tempsVals
    {20,24,27,30,33,36,40},
    // fansVals
    {45,70,100,140,200,220,255}
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
void setFanSpeedFromStage(struct StageCfg stage){
  if(stage.tempsSize < 0){
    float temperature = readDHTTemperature();
    for(int i=0; i<stage.tempsSize;i++){
      if(temperature < stage.tempsVals[i]){
          setPwmVal(stage.fansVals[i], NLIGHTS);
          Serial.println( String(i)+" setting: fan to"+String(stage.fansVals[i])+" T"+String(temperature) );
          break;
        }
    }
    setPwmVals(stage_off.pwmVals);
  } else {
    Serial.println("no temps to  set");
    //setPwmVal(stage.pwmVals, NLIGHTS);
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
  setFanSpeedFromStage(selStageCfg);
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
  timerAlarmWrite(timer, 5*600000, true);
  timerAlarmEnable(timer);
  
  timerHandler();
  
}
