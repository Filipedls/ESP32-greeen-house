#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Aux Vars for http calls
int sliderValueAux = 0;
const char* PARAM_INPUT = "value";
int hour_on_slide_id = 420;
int hours_off_slide_id = 421;

int temp_offset_slide_id = 422;

char slidarValsChar[100];

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
     background-color: lightgray;
    }
    input { background-color: lightgray; width: min(50vw, 220px); }
    h2 { font-size: 2.0rem; }
    p { font-size: 1.5rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.3rem;
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
  <h3>Greenkea</h3>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup> 
    <i class="fas fa-tint" style="color:#00add6;"></i> 
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">&percnt;</sup>
    <button type="button" onclick="updateSensorsSliders(this)">R</button>
  </p>
  <h6>%DATETIME%</h6>
  <h4>Lights</h4>
  %SLIDERSPLACEHOLDER%
  <h4>Modes</h4>
  <h5>light mode: %DROPDOWNPLACEHOLDER%</h5>
  %HOURONOFF%
  <h5>fan mode: %DROPDOWNFANPLACEHOLDER%</h5>
  %FANTEMPOFFSET%
  %WARNS%
  <br>
  <details>
    <summary>info</summary>
    <br>
    %INFO%
    <button type="button" onclick="buttonRestart(this)">Restart ESP</button>
  </details>
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

function dowpdownChanged(element, update_url) {
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      updateSensorsSliders();
    }
  };
  xhr.open("GET", "/"+update_url+"?state="+element.value, true);
  xhr.send();
}

function updateSensorsSliders(elem) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      var all_vals = this.responseText.split(";");
      // temp hum
      var sensor_vals = all_vals[0].split(",");
      document.getElementById("temperature").innerHTML = sensor_vals[0];
      document.getElementById("humidity").innerHTML = sensor_vals[1];
      // slider
      for (var x = 1; x < all_vals.length;x++) {
        var id_vals = all_vals[x].split(":");
        document.getElementById("textSliderValueLight"+id_vals[0]).innerHTML = id_vals[1];
        document.getElementById("pwmSlider"+id_vals[0]).value = id_vals[1];
      }
      if(elem != undefined){elem.style.backgroundColor = null;}
    }
  };
  xhttp.open("GET", "/getsenslivals", true);
  xhttp.send();
  if(elem != undefined){elem.style.backgroundColor = "red";}

}
//setInterval(updateSensorsSliders, 60000 ) ;

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
}
console.log("done setuping");
</script>
</html>
)rawliteral";

// Replaces placeholder with DHT values
String processor(const String& var){
  //Serial.println("processor "+var);
  if(var == "TEMPERATURE"){
    return String(readDHTTemperature());
  }
  else if(var == "HUMIDITY"){
    return String(readDHTHumidity());
  } else if (var == "SLIDERSPLACEHOLDER"){
    String buttons ="";
    pwmValsInfo sliderInfo = getPwmVals();
    for(int i=0; i < sliderInfo.lenght; i++){
      String sliderVal = String(sliderInfo.vals[i]);
      if(i == sliderInfo.lenght-NFANS){
        buttons+="<h4>Fan</h4>";
      }
      buttons+= "<h5>" + String(sliderInfo.pwmNames[i]) + ": <input type=\"range\" onchange=\"updateSliderPWM(this, '" + String(i) +
      "')\" oninput=\"onslideSliderPWM(this, '" + String(i) + "')\" id=\"pwmSlider" + String(i) +"\" min=\"0\" max=\"255\" value=\""+ sliderVal + 
      "\" step=\"15\" class=\"sliderlight\"><span id=\"textSliderValueLight"+ String(i) +"\">"+sliderVal+"</span></h5>";
    }
    return buttons;
  } else if (var == "DATETIME"){
    return getDateTime();
  } else if(var == "DROPDOWNPLACEHOLDER"){
    String buttons ="<select id=\"stagesdropdown\" onchange=\"dowpdownChanged(this,'updatestage')\">";
    int stateValue = getStage();
    for(int i=0; i<NSTAGES; i++){
      buttons+= "<option value=\""+String(i)+"\" "+String(stateValue==i?"selected=\"selected\"":"")+
      ">"+String(i+1)+". "+String(all_modes[i].sname)+"</option>";
    }
    buttons+= "</select>";
    return buttons;
  } else if(var == "DROPDOWNFANPLACEHOLDER"){
    String buttons ="<select id=\"fandropdown\" onchange=\"dowpdownChanged(this,'updatefanstate')\">";
    int fanValue = getTempConfigN();
    for(int i=0; i<NTEMPCONFIGS; i++){
      buttons+= "<option value=\""+String(i)+"\" "+String(fanValue==i?"selected=\"selected\"":"")+
      ">"+String(i+1)+". "+String(all_temp_configs[i]->tname)+"</option>";
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
  } else if(var == "FANTEMPOFFSET"){
    String temp_offset_id = String(temp_offset_slide_id);
    String mid_fan_speed_temp_str = String(mid_fan_speed_temp);
    String text = "<h5>T: <input type=\"range\" onchange=\"updateSliderPWM(this, '" + temp_offset_id +
      "')\" oninput=\"onslideSliderPWM(this, '" + temp_offset_id + "')\" id=\"pwmSlider" + temp_offset_id +"\" min=\"20\" max=\"33\" value=\""+ mid_fan_speed_temp_str + 
      "\" step=\"1\" class=\"sliderlight\"><span id=\"textSliderValueLight"+ temp_offset_id +"\">"+mid_fan_speed_temp_str+"</span>&deg;C</h5>";
     // hours off
    return text;
  } else if(var == "WARNS") {
    String text = "<span style=\"color:red\">";
    if(n_times_temp_nan > 2)
      text += "temp is nan 3x!<br>";
    if(max_fan_speed_warn)
      text += "Temperature above the allowed max, reseting the max fan speed!<br>";
    text += "</span>";
    return text;
  } else if(var == "INFO") {
    String text = "lights off at: "+String(max_temp_lights)+"&deg;C<br>"+
      "fan speed (min-max): "+String(min_fan_speed)+"-"+String(max_fan_speed)+
      "<br>no max fan speed temp: "+String(no_max_fan_speed_temp)+"&deg;C<br>";
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

  // Send a the value of the sensors and light /getsenslivals
  server.on("/getsenslivals", HTTP_GET, [] (AsyncWebServerRequest *request) {
    float t = NAN;
    float h = NAN;
    readDHTTemperatureHumidity(&t, &h);
    sprintf(slidarValsChar, "%.2f,%.2f", t,h);
    
    pwmValsInfo sliderInfo = getPwmVals();
    for(int i=0; i < sliderInfo.lenght; i++){
      //slidarValsChar+= String(sliderInfo.vals[i]);
      sprintf(slidarValsChar, "%s;%i:%i", slidarValsChar, i, sliderInfo.vals[i]);
    }
    
    StageCfg selStageCfg = getSelectedStage();
    sprintf(slidarValsChar, "%s;%i:%i", slidarValsChar, hour_on_slide_id, selStageCfg.hour_on);
    sprintf(slidarValsChar, "%s;%i:%i", slidarValsChar, hours_off_slide_id, selStageCfg.n_hours_off);
    
    //Serial.println("sv "+String(slidarValsChar)+" - "+selStageCfg.sname+" "+String(selStageCfg.hour_on)+String(selStageCfg.n_hours_off));
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
      } else if (sliderID == temp_offset_slide_id) {
        setDynamicTemp(sliderValueAux);  
      } else if(sliderID==NLIGHTS){//  if main fan, staps AutoFAn control
        setMainFanPwm(sliderValueAux);
        //stopAutoFan();
        //Serial.println("pwmID FAN stopAutoFan");
      } else {
        setPwmLight(sliderID, sliderValueAux);
        //Serial.println("Light "+ String(sliderID)+" V" + String(sliderValueAux));
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
