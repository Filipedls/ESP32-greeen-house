//#include "00_pwm_utils"
//#include "01_stages"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Aux Vars for http calls
int sliderValueAux = 0;
const char* PARAM_INPUT = "value";
int hour_on_slide_id = 420;
int hours_off_slide_id = 421;

char slidarValsChar[28];

int sliderID = 0;
const char* PARAM_INPUT_ID = "id";

int getSliderID = 0;

const char* PARAM_INPUT_CB_STATE = "state";


// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href='https://use.fontawesome.com/releases/v5.7.2/css/all.css' integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 2.0rem; }
    p { font-size: 1.5rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
    .switch {position: relative; display: inline-block; width: 60px; height: 34px} 
    .switch input {display: none}
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #ccc; border-radius: 34px}
    .slider:before {position: absolute; content: ""; height: 26px; width: 26px; left: 4px; bottom: 4px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 68px}
    input:checked+.slider {background-color: #2196F3}
    input:checked+.slider:before {-webkit-transform: translateX(26px); -ms-transform: translateX(26px); transform: translateX(26px)}
  </style>
</head>
<body>
  <h2>Green Settings</h2>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup> 
    <i class="fas fa-tint" style="color:#00add6;"></i> 
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">&percnt;</sup>
    <button type="button" onclick="updateTempHumd(this)">R</button>
  </p>
  <h5>%DATETIME%</h5>
  <h3>Lights</h3>
  %SLIDERSPLACEHOLDER%
  <h3>Modes</h3>
  <h5>grow stage</h5>
  %DROPDOWNPLACEHOLDER%
  %HOURONOFF%
  <h5>smart fan mode</h5>
  %DROPDOWNFANPLACEHOLDER%
  <br><br>
  <button type="button" onclick="buttonRestart(this)">Restart ESP</button>
</body>
<script>
function onslideSliderPWM(element, slider_id) {
  var sliderValue = element.value;
  var textSlider = document.getElementById("textSliderValueLight"+slider_id);
  textSlider.innerHTML = sliderValue;
  textSlider.style.color = "red";
}

function updateSliderPWM(element, slider_id) {
  var sliderValue = element.value;
  document.getElementById("textSliderValueLight"+slider_id).innerHTML = sliderValue;
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("textSliderValueLight"+slider_id).style.color = null;
    }
  };
  xhr.open("GET", "/slider?value="+sliderValue+"&id="+slider_id, true);
  xhr.send();
}

function toggleCheckbox(element) {
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      getAllLightVal();
    }
  };
  if(element.checked){ xhr.open("GET", "/updatestage?state=1", true); }
  else { xhr.open("GET", "/updatestage?state=0", true); }
  xhr.send();
}


function dowpdownChanged(element) {
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      getAllLightVal();
    }
  };
  xhr.open("GET", "/updatestage?state="+element.value, true);
  xhr.send();
}

function fanDowpdownChanged(element) {
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      getAllLightVal();
    }
  };
  xhr.open("GET", "/updatefanstate?state="+element.value, true);
  xhr.send();
}

function updateTempHumd(elem) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var sensor_vals = this.responseText.split(",");
      document.getElementById("temperature").innerHTML = sensor_vals[0];
      document.getElementById("humidity").innerHTML = sensor_vals[1];
      elem.style.backgroundColor = null;
    }
    // TODO: one call to rule them all
    getAllLightVal();
  };
  xhttp.open("GET", "/getsensorvals", true);
  xhttp.send();
  elem.style.backgroundColor = "red";
}
//setInterval(updateTempHumd, 60000 ) ;

function getAllLightVal() {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var light_vals = this.responseText.split(",");
      for (var x = 0; x < light_vals.length;x++) {
        document.getElementById("textSliderValueLight"+x).innerHTML = light_vals[x];
        document.getElementById("pwmSlider"+x).value = light_vals[x];
      }
    }
  };
  xhttp.open("GET", "/slidervalues", true);
  xhttp.send();
}

function buttonRestart(elem) {
  if (confirm("Are you sure?")) {
    elem.style.backgroundColor = "red";
    var xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        console.log("restarted");
        elem.style.backgroundColor = "green";
        setInterval(function ( ) {
          elem.style.backgroundColor = null;
        }, 1000);                                        
      }
    };
    xhr.open("GET", "/restartesp", true);
    xhr.send();
  }
}//setTimeout(getAllLightVal, 600000);
//getAllLightVal();
console.log("done setuping");
</script>
</html>
)rawliteral";

String switchStateStr(){
    if(getStage()){
      return "checked";
    }
    else {
      return "";
    }
}

// Replaces placeholder with DHT values
String processor(const String& var){
  //Serial.println("processor "+var);
  if(var == "TEMPERATURE"){
    return String(readDHTTemperature());
  }
  else if(var == "HUMIDITY"){
    return String(readDHTHumidity());
  } else if(var == "BUTTONPLACEHOLDER"){
    String buttons ="";
    for(int i=1; i<=1; i++){
      String stateValue = switchStateStr();
      buttons+= "<h4><label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"" 
      + String(i) + "\" "+ stateValue +"><span class=\"slider\"></span></label></h4>";
    }
    return buttons;
  } else if (var == "SLIDERSPLACEHOLDER"){
    String buttons ="";
    pwmValsInfo sliderInfo = getPwmVals();
    for(int i=0; i < sliderInfo.lenght; i++){
      String sliderVal = String(sliderInfo.vals[i]);
      if(i == sliderInfo.lenght-NFANS){
        buttons+="<h3>Fan</h3>";
      }
      buttons+= "<h4>" + String(sliderInfo.pwmNames[i]) + ": <input type=\"range\" onchange=\"updateSliderPWM(this, '" + String(i) +
      "')\" oninput=\"onslideSliderPWM(this, '" + String(i) + "')\" id=\"pwmSlider" + String(i) +"\" min=\"0\" max=\"255\" value=\""+ sliderVal + 
      "\" step=\"15\" class=\"sliderlight\"><span id=\"textSliderValueLight"+ String(i) +"\">"+sliderVal+"</span></h4>";
    }
    return buttons;
  } else if (var == "DATETIME"){
    return getDateTime();
  } else if(var == "DROPDOWNPLACEHOLDER"){
    String buttons ="<select id=\"stagesdropdown\" onchange=\"dowpdownChanged(this)\">";
    int stateValue = getStage();
    for(int i=0; i<NSTAGES; i++){
      buttons+= "<option value=\""+String(i)+"\" "+String(stateValue==i?"selected=\"selected\"":"")+
      ">"+String(i+1)+". "+String(all_modes[i].sname)+"</option>";
    }
    buttons+= "</select>";
    return buttons;
  } else if(var == "DROPDOWNFANPLACEHOLDER"){
    String buttons ="<select id=\"fandropdown\" onchange=\"fanDowpdownChanged(this)\">";
    int fanValue = getTempConfigN();
    for(int i=0; i<NTEMPCONFIGS; i++){
      buttons+= "<option value=\""+String(i)+"\" "+String(fanValue==i?"selected=\"selected\"":"")+
      ">"+String(i+1)+". "+String(all_temp_configs[i].tname)+"</option>";
    }
    buttons+= "</select>";
    return buttons;
  } else if(var == "HOURONOFF"){
    String hour_on_id = String(hour_on_slide_id);
    String hours_off_id = String(hours_off_slide_id);
    StageCfg selStageCfg = getSelectedStage();
    String text = "<h5>H on: <input type=\"range\" onchange=\"updateSliderPWM(this, '" + hour_on_id +
      "')\" oninput=\"onslideSliderPWM(this, '" + hour_on_id + "')\" id=\"pwmSlider" + hour_on_id +"\" min=\"0\" max=\"23\" value=\""+ selStageCfg.hour_on + 
      "\" step=\"1\" class=\"sliderlight\"><span id=\"textSliderValueLight"+ hour_on_id +"\">"+selStageCfg.hour_on+"</span></h5>";
     // hours off
     text += "<h5>Hs off: <input type=\"range\" onchange=\"updateSliderPWM(this, '" + hours_off_id +
      "')\" oninput=\"onslideSliderPWM(this, '" + hours_off_id + "')\" id=\"pwmSlider" + hours_off_id +"\" min=\"0\" max=\"24\" value=\""+ selStageCfg.n_hours_off + 
      "\" step=\"1\" class=\"sliderlight\"><span id=\"textSliderValueLight"+ hours_off_id +"\">"+selStageCfg.n_hours_off+"</span></h5>";
    return text;
  }
  return String();
}

void setupServer(){
   // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/restartesp", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "OK");
    delay(1000);
    ESP.restart();
  });
  server.on("/getsensorvals", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readDHTTemperatureHumidity().c_str());
  });

  // Send a the value of a light /slidervalue
  server.on("/slidervalues", HTTP_GET, [] (AsyncWebServerRequest *request) {
    // GET input1 value on <ESP_IP>/slidervalues
    pwmValsInfo sliderInfo = getPwmVals();

    //slidarValsChar = "";
    sprintf(slidarValsChar, "%i", sliderInfo.vals[0]);
    for(int i=1; i < sliderInfo.lenght; i++){
      //slidarValsChar+= String(sliderInfo.vals[i]);
      sprintf(slidarValsChar, "%s,%i", slidarValsChar, sliderInfo.vals[i]);
//      if(i != sliderInfo.lenght-1){
//        slidarValsChar+=",";
//      }
    }
//    sprintf(slidarValsChar, "%i,%i,%i,%i,%i,%i", 
//      sliderInfo.vals[0], sliderInfo.vals[1], sliderInfo.vals[2],
//      sliderInfo.vals[3], sliderInfo.vals[4], sliderInfo.vals[5]);

    request->send_P(200, "text/plain", slidarValsChar);
  });

  // Send a GET request to <ESP_IP>/slider?value=<inputMessage>
  server.on("/slider", HTTP_GET, [] (AsyncWebServerRequest *request) {
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT)) {
      sliderValueAux = request->getParam(PARAM_INPUT)->value().toInt();
      sliderID = request->getParam(PARAM_INPUT_ID)->value().toInt();
      if(sliderID == hour_on_slide_id){
        setHourOn(sliderValueAux);
      } else if(sliderID == hours_off_slide_id){
        setNHoursOff(sliderValueAux);
      } else if(sliderID==NLIGHTS){//  if main fan, staps AutoFAn control
        setMainFanPwm(sliderValueAux);
        stopAutoFan();
        //Serial.println("pwmID FAN stopAutoFan");
      } else {
        setPwmVal(sliderID, sliderValueAux);
        Serial.println("pwmID "+ String(sliderID)+" V" + String(sliderValueAux));
      }
    }
    else {
      Serial.println("No message sent");
    }
    request->send(200, "text/plain", "OK");
  });
  
  // Send a GET request to <ESP_IP>/updatestage?state=<inputMessage2>
  server.on("/updatestage", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    // GET input1 value on <ESP_IP>/update?relay=<inputMessage>
    if (request->hasParam(PARAM_INPUT_CB_STATE)) {
      inputMessage = request->getParam(PARAM_INPUT_CB_STATE)->value();
      setStage(inputMessage.toInt());
    }
    else {
      Serial.println("CB No message sent");
    }
    request->send(200, "text/plain", "OK");
  });
  // Send a GET request to <ESP_IP>/updatefanstate?state=<inputMessage2>
  server.on("/updatefanstate", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    // GET input1 value on <ESP_IP>/update?relay=<inputMessage>
    if (request->hasParam(PARAM_INPUT_CB_STATE)) {
      inputMessage = request->getParam(PARAM_INPUT_CB_STATE)->value();
      setTempConfig(inputMessage.toInt());
    }
    else {
      Serial.println("CBfan No message sent");
    }
    request->send(200, "text/plain", "OK");
  });
  // Start server
  server.begin();
}
