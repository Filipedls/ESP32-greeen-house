//#include "00_pwm_utils"
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Aux Vars for http calls
int sliderValueAux = 0;
const char* PARAM_INPUT = "value";

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
  <h2>Green Control Room</h2>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup> 
    <i class="fas fa-tint" style="color:#00add6;"></i> 
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">&percnt;</sup>
  </p>
  <h3>Lights</h3>
  %SLIDERSPLACEHOLDER%
  <h3>Mode selector</h3>
  <h4>1 %BUTTONPLACEHOLDER% 2</h4>
</body>
<script>
function updateSliderPWM(element, slider_id) {
  var sliderValue = element.value;
  document.getElementById("textSliderValueLight"+slider_id).innerHTML = sliderValue;
  var xhr = new XMLHttpRequest();
  xhr.open("GET", "/slider?value="+sliderValue+"&id="+slider_id, true);
  xhr.send();
}

function toggleCheckbox(element) {
  var xhr = new XMLHttpRequest();
  if(element.checked){ xhr.open("GET", "/updatecheckbox?state=1", true); }
  else { xhr.open("GET", "/updatecheckbox?state=0", true); }
  xhr.send();
}

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 60000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 60000 ) ;

function getLightVal(slider_id) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("textSliderValueLight"+slider_id).innerHTML = this.responseText;
      document.getElementById("pwmSlider"+x).value = this.responseText;
    }
  };
  xhttp.open("GET", "/slidervalue?id="+slider_id, true);
  xhttp.send();    
  console.log("interval id: %d", slider_id);
}
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
  xhttp.open("GET", "/slidervalue", true);
  xhttp.send();    
  console.log("interval all");
}
//setTimeout(getAllLightVal, 600000);
//getAllLightVal();
//timer for light vals
//for (var x = 0; x < 6; x++) {
//  setTimeout(getLightVal, 55000, x); // x - slider_id
//  getLightVal(x);
//}
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
    return readDHTTemperature();
  }
  else if(var == "HUMIDITY"){
    return readDHTHumidity();
  } else if(var == "BUTTONPLACEHOLDER"){
    String buttons ="";
    for(int i=1; i<=1; i++){
      String relayStateValue = switchStateStr();
      buttons+= "<label class=\"switch\"><input type=\"checkbox\" onchange=\"toggleCheckbox(this)\" id=\"" 
      + String(i) + "\" "+ relayStateValue +"><span class=\"slider\"></span></label>";
    }
    return buttons;
  } else if (var == "SLIDERSPLACEHOLDER"){
    String buttons ="";
    SliderValsInfo sliderInfo = getSliderVals();
    for(int i=0; i < sliderInfo.lenght; i++){
      String sliderVal = String(sliderInfo.vals[i]);
      if(i == sliderInfo.lenght-1){
        buttons+="<h3>Fan</h3>";
      }
      buttons+= "<h4>" + String(i) + ": <input type=\"range\" onchange=\"updateSliderPWM(this, '" + String(i) +
      "')\" id=\"pwmSlider" + String(i) +"\" min=\"0\" max=\"255\" value=\""+ sliderVal + 
      "\" step=\"1\" class=\"sliderlight\"><span id=\"textSliderValueLight"+ String(i) +"\">"+sliderVal+"</span></h4>";
    }
    return buttons;
  }
  return String();
}

void setupServer(){
   // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readDHTTemperature().c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", readDHTHumidity().c_str());
  });

  // Send a the value of a light /slidervalue?id=<lightid>
  server.on("/slidervalue", HTTP_GET, [] (AsyncWebServerRequest *request) {
    // GET input1 value on <ESP_IP>/slidervalue?id=<inputMessage>
    SliderValsInfo sliderInfo = getSliderVals();
    if (request->hasParam(PARAM_INPUT_ID)) {
      getSliderID = request->getParam(PARAM_INPUT_ID)->value().toInt();

      request->send_P(200, "text/plain", String(sliderInfo.vals[getSliderID]).c_str());
      Serial.println("gL"+String(getSliderID)+" V"+String(sliderInfo.vals[getSliderID]));
    }
    else {
      sprintf(slidarValsChar, "%i,%i,%i,%i,%i,%i", 
        sliderInfo.vals[0], sliderInfo.vals[1], sliderInfo.vals[2],
        sliderInfo.vals[3], sliderInfo.vals[4], sliderInfo.vals[5]);

      request->send_P(200, "text/plain", slidarValsChar);
      Serial.println("agL "+String(slidarValsChar));
    }
  });

  // Send a GET request to <ESP_IP>/slider?value=<inputMessage>
  server.on("/slider", HTTP_GET, [] (AsyncWebServerRequest *request) {
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT)) {
      sliderValueAux = request->getParam(PARAM_INPUT)->value().toInt();
      sliderID = request->getParam(PARAM_INPUT_ID)->value().toInt();
      setSliderVal(sliderID, sliderValueAux);
      Serial.println("L"+String(sliderID)+" V"+String(sliderValueAux));
    }
    else {
      Serial.println("No message sent");
    }
    request->send(200, "text/plain", "OK");
  });
  
  // Send a GET request to <ESP_IP>/updatecheckbox?state=<inputMessage2>
  server.on("/updatecheckbox", HTTP_GET, [] (AsyncWebServerRequest *request) {
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
  // Start server
  server.begin();
}
