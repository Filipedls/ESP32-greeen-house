const int resolution = 8;

/// PWM CONFIG

// setting PWM properties for light
const int freq = 300;
//const int ledChannel = 0;
// the number of the LED pin
//const int ledPin = 27;  // 16 corresponds to GPIO16

int nLights =  5;
int ledChannelVals[5] = {0, 1, 2, 3, 4};
int ledPinsVals[5] = {27, 1, 2, 3, 4};

// Setting pwm for fan
// https://www.mikrocontroller.net/attachment/361299/noctua_nf_a14_industrialPPC_3000_pwm_specs_en.pdf
const int freq_fan = 10000;
const int channel_fan = 5;
const int pin_fan = 26;

// This initialises the six servos
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
#define NSLIDERS 6 
int sliderVals[NSLIDERS] = {0, 10, 20, 30, 40, 44};
struct SliderValsInfo {
    int vals[NSLIDERS];
    size_t lenght;
};


struct SliderValsInfo getSliderVals(){
  SliderValsInfo sliderInfo;
  for(int i=0; i<NSLIDERS;i++){
    sliderInfo.vals[i] = sliderVals[i];
  }
  sliderInfo.lenght = NSLIDERS;
  return sliderInfo;
}


void setSliderVal(int sliderID, int val){
  ledcWrite(sliderID, val);
  sliderVals[sliderID] = val;
}


void  setupPWM(){
  // configure LED PWM functionalitites
  for(int i = 0; i <= 0; i++){   // nLights
    PinSetup(freq, ledChannelVals[i], ledPinsVals[i]);
    ledcWrite(ledChannelVals[i], 0);
  }
  //PinSetup(freq, ledChannel, ledPin);
  //ledcWrite(ledChannel, 0);
  // fan PWM
  PinSetup(freq_fan, channel_fan, pin_fan);
  ledcWrite(channel_fan, 0);
}
