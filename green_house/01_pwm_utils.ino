const int resolution = 8;

/// PWM CONFIG

#define NLIGHTS 5
#define NFANS 1
#define NPWMS (NLIGHTS + NFANS)

int pwmChannels[NPWMS] = {0, 1, 2, 3, 4, 5};
int pwmPins[NPWMS] = {19, 2, 4, 5, 18, 23};
// others: 15, 27, 19, 21
int pwmVals[NPWMS] = {0, 0, 0, 0, 0, 0};

// setting PWM properties for light
const int freq_lights = 300;

// Setting pwm properties for fan
// https://www.mikrocontroller.net/attachment/361299/noctua_nf_a14_industrialPPC_3000_pwm_specs_en.pdf
const int freq_fans = 10000;


void PinSetup(int freq, int channel, int pin) {
  // configure PWM functionalitites
  ledcSetup(channel, freq, resolution);
  // attach the channel to the GPIO to be controlled
  ledcAttachPin(pin, channel);
}

//
void PinWrite(int channel, int dutyCycle) {
  ledcWrite(channel, dutyCycle);
}

// server methods
struct pwmValsInfo {
    const char pwmNames[NPWMS][2] = {"R", "L", "F", "B", "r", "f"};
    int vals[NPWMS];
    size_t lenght;
};


struct pwmValsInfo getPwmVals(){
  pwmValsInfo pwmInfo;
  for(int i=0; i<NPWMS;i++){
    pwmInfo.vals[i] = pwmVals[i];
  }
  pwmInfo.lenght = NPWMS;
  //pwmInfo.lightNames = lightNames;
  return pwmInfo;
}


void setPwmVal(int pwmID, int val){
  ledcWrite(pwmID, val);
  pwmVals[pwmID] = val;
}


void setPwmVals(const int vals[]){
  for(int i=0; i<NPWMS;i++){
    setPwmVal(i, vals[i]);
  }
}

void  setupPWM(){
  // configure LED PWM functionalitites
  for(int i = 0; i < NLIGHTS; i++){
    PinSetup(freq_lights, pwmChannels[i], pwmPins[i]);
    ledcWrite(pwmChannels[i], 0);
  }
  // configure Fan PWM functionalitites
  for(int i = NLIGHTS; i < NPWMS; i++){
    PinSetup(freq_fans, pwmChannels[i], pwmPins[i]);
    ledcWrite(pwmChannels[i], 0);
  }
}
