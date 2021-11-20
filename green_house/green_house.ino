// Import required libraries
// include library to read and write from flash memory
#include <EEPROM.h>
#define ELEMENTS(x)   (sizeof(x) / sizeof(x[0]))

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  // one byte for the sel stage num, another for the sel temp
  // EEPROM_STAGE_SIZE+EEPROM_T_SIZE
  EEPROM.begin(3);
  // Setup's  `x)
  setupPWM();
  setupDHT();
  setupWiFi();
  setupTime(); // needs setupWiFi
  //setupLogs();
  setupStages(); // needs setupTime
  setupFan(); 
  setupServer(); // needs setupTime, setupStages
  // wait 5s for the temp sensor
  delay(5000);
}
 
void loop(){
  for(int i=1; i<11;i++){
    updateFanSpeed();
//    if(i==1)
//      logTempHumidToGS();
      
    delay(2*60000);
  }
}
