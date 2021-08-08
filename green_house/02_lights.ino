// setting PWM properties for light
const int freq_lights_ = 300;

float interpolate_xy(float x1, float x2, float y1, float y2, float xpoint){
  return y2-(y2-y1) * ((x2-xpoint)/(x2-x1));
}

class LightProfile{
  private:
    String pname;
    int psize;
    int *pwms;
    float *hours;
    int prev_pwm;
    int min_pwm;
    int pwm_channel;
  public:
    LightProfile (String pname, int psize, int pwms[], float hours[], int pwm_channel, int pwm_pin){
      this->pname = pname;
      this->psize = psize;
      this->pwms = pwms;
      this->hours = hours;
      prev_pwm = 0;
      min_pwm = 8;
      this->pwm_channel = pwm_channel;

      //PinSetup(freq_lights_, pwm_channel, pwm_pin);
      //setPwmVal(pwm_channel, 0);
      Serial.println("LP: init "+pname+" Chn:"+String(pwm_channel)+" Pin:"+String(pwm_pin));
    }
    void pinfo(void){
      Serial.print("LP: "+pname);
      for(int i=0; i<psize;i++){
        Serial.print(" "+String(hours[i]) +"h P"+String(pwms[i]) );
      }
      Serial.println(";");
    }
    void updateLight(int hour, int mins){
      
      float hour_min = hour + mins/60.0;

      Serial.println("LP: "+String(hour)+"h"+String(mins)+" F:"+String(hour_min));
      int pwmVal;

      // TODO generalize to y2-(y2-y1) * ((x2-xpoint)/(x2-x1))
      if(hour_min < hours[0]){
//        pwmVal = pwms[0]-(pwms[0]-pwms[psize-1])*
//          ((hours[0]-hour_min)/(hours[0]+24.0-hours[psize-1]));
          Serial.println("LP: 1st");
          pwmVal = interpolate_xy(hours[psize-1]-24.0, hours[0], pwms[psize-1], pwms[0], hour_min);
      // above last hour
      } else if(hour_min >= hours[psize-1]) {
//        pwmVal = pwms[0]-(pwms[0]-pwms[psize-1])*
//          ((hours[0]+24.0-hour_min)/(hours[0]+24.0-hours[psize-1]));

          Serial.println("LP: 2nd");
          pwmVal = interpolate_xy(hours[psize-1], hours[0]+24.0, pwms[psize-1], pwms[0], hour_min);
      // hour is somewheree in thee midle
      } else {
        for(int i=1; i<psize;i++){
        
          if(hour_min < hours[i]){
          
//            pwmVal = pwms[i]-(pwms[i]-pwms[i-1])*
//                ((hours[i]-hour_min)/(hours[i]-hours[i-1]));
            
            pwmVal = interpolate_xy(hours[i-1], hours[i], pwms[i-1], pwms[i], hour_min);
             
            Serial.println("LP: final i "+ String(i) );
            break;
          }
        }
      }
      if(pwmVal < min_pwm)
        pwmVal = 0;
      
      if(pwmVal != prev_pwm)
        Serial.println("LP: diff final pwm "+String(pwmVal));
      else
        Serial.println("LP: same final pwm "+String(pwmVal));
      // TODO replace by ledcWrite
      //setPwmVal(pwm_channel, pwmVal);
      prev_pwm = pwmVal;
    }
};

// LightProfile led_profile(String("P1"), 4, {0, 255, 255, 0}, {1.0, 12.0, 13.0, 23.0});

int lp1_pwm[] = {0, 0, 255, 255};
float lp1_hours[] = {13.0, 14.0, 20.0, 21.0};
//LightProfile led_profile = LightProfile(String("R"), 4, lp1_pwm, lp1_hours, 0, 19);

#define NTLIGHTS 2
LightProfile all_led_profiles[NTLIGHTS] = {
  // Right
  LightProfile(String("R"), 4, lp1_pwm, lp1_hours, 0, 19),
  // Left
  LightProfile(String("L"), 4, lp1_pwm, lp1_hours, 1, 2)
};

void updateLightProfiles(){
  int hour, mins;
  getHourMins(&hour, &mins);
  for(int i = 0; i < NTLIGHTS; i++){
    all_led_profiles[i].updateLight(hour, mins);
  }
}

void setupLightProfiles(){
  Serial.println("LP: setuping all");
  //led_profile.pinfo();
  for(int i = 0; i < NTLIGHTS; i++){
    all_led_profiles[i].pinfo();
  }
  //updateLightProfiles();
}
