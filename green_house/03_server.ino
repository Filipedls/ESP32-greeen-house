
// TOCHANGE
//   * create lights and fan section with the respective info
//   * hide sliders
//   * add a middle div around the controls with a different color (vertical style)
//   * generalize the processor() method. eg: a class to render sliders, dropdown's, etc
//   * cmd to change light stage name; and add/remove stages?

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

// HTML form Config
const char* PARAM_INPUT_CFGSEL = "cfgsel";
const char* PARAM_INPUT_CFGVAL = "cfgval";

String server_started_at;

String off_color = "#444444";
String on_color  = "#059e8a";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

const uint8_t favicon_ico_gz[] PROGMEM = {
  0x1f, 0x8b, 0x08, 0x08, 0x61, 0x94, 0x36, 0x64, 0x00, 0x03, 0x66, 0x61,
  0x76, 0x69, 0x63, 0x6f, 0x6e, 0x2e, 0x69, 0x63, 0x6f, 0x00, 0x63, 0x60,
  0x60, 0x04, 0x42, 0x01, 0x01, 0x06, 0x20, 0xa9, 0xc0, 0x90, 0xc1, 0xc2,
  0xc0, 0x20, 0xc6, 0xc0, 0xc0, 0xa0, 0x01, 0xc4, 0x40, 0x21, 0xa0, 0x08,
  0x44, 0x1c, 0x0c, 0x80, 0x72, 0xc2, 0xdc, 0x10, 0x4c, 0x0f, 0xe0, 0x71,
  0xb8, 0xdd, 0x97, 0xe1, 0xff, 0x7f, 0x46, 0x72, 0xf5, 0x7b, 0x1d, 0xea,
  0xbe, 0xef, 0x73, 0xa6, 0x47, 0x84, 0x58, 0xf5, 0x36, 0xdb, 0x2a, 0xb5,
  0xac, 0x96, 0xe6, 0x7e, 0xb1, 0x5c, 0x98, 0xf5, 0xcf, 0x79, 0x73, 0x4d,
  0xa3, 0xd7, 0xe1, 0xee, 0xcd, 0x6e, 0xfb, 0x9a, 0x67, 0x5a, 0x2e, 0xca,
  0xfa, 0x0b, 0x14, 0xff, 0xee, 0xb8, 0xb1, 0x46, 0x1b, 0x9f, 0x7e, 0xdb,
  0x95, 0x45, 0x17, 0x2d, 0xe6, 0x67, 0xfc, 0x07, 0x61, 0x87, 0x8d, 0x55,
  0x1f, 0xfc, 0xf7, 0xf7, 0x0b, 0xb8, 0xee, 0x6e, 0x5c, 0x6b, 0xb5, 0x38,
  0x07, 0x2c, 0x66, 0xbb, 0x3c, 0xff, 0x25, 0x2e, 0xbd, 0x4e, 0x1b, 0x5b,
  0xc5, 0x2d, 0x16, 0x66, 0xfe, 0xb3, 0x5e, 0x9a, 0xfb, 0xdf, 0x75, 0x67,
  0xe3, 0x7f, 0xa0, 0xdd, 0xff, 0x3d, 0x0e, 0xf7, 0x5a, 0x78, 0x1d, 0xee,
  0x9a, 0x0d, 0xc4, 0xff, 0xdd, 0x76, 0x37, 0xff, 0x77, 0xdc, 0x58, 0xf5,
  0xdf, 0x69, 0x63, 0x4d, 0x31, 0x36, 0xfd, 0xce, 0x9b, 0xab, 0x1b, 0x5c,
  0xb6, 0x37, 0xfc, 0x06, 0xa9, 0x05, 0xe9, 0xf5, 0x3c, 0xdc, 0xfd, 0xce,
  0xed, 0x60, 0x8f, 0xac, 0xc7, 0x91, 0x2e, 0x65, 0x20, 0x7f, 0x9a, 0xe7,
  0xe1, 0xae, 0xa9, 0x5e, 0x47, 0x3a, 0x1d, 0xf1, 0xb9, 0xdf, 0xe3, 0xc4,
  0x44, 0x3e, 0xcf, 0x83, 0xdd, 0x4e, 0x5e, 0x07, 0xbb, 0xbd, 0xdd, 0x8e,
  0xf5, 0x0a, 0x11, 0x17, 0x6a, 0x44, 0x80, 0xe9, 0xff, 0x15, 0x80, 0xf8,
  0x00, 0x98, 0x26, 0x15, 0xcc, 0xf8, 0xbf, 0x95, 0x79, 0xce, 0xff, 0xff,
  0x82, 0xeb, 0xff, 0xff, 0x07, 0xd1, 0x40, 0x33, 0xb6, 0x11, 0xad, 0x77,
  0xe6, 0x7f, 0x39, 0xa0, 0xfe, 0x2f, 0x7a, 0x37, 0xff, 0xff, 0xb7, 0x7e,
  0xfd, 0xff, 0xbf, 0xde, 0x0d, 0xb0, 0xfe, 0xaf, 0x0c, 0xd3, 0xfe, 0x2b,
  0x11, 0x69, 0x77, 0x10, 0x50, 0xfd, 0x3f, 0xb3, 0xa7, 0x10, 0xfd, 0x66,
  0x4f, 0xc0, 0xfa, 0xff, 0x81, 0xc5, 0x89, 0x77, 0xff, 0x71, 0xfe, 0xb5,
  0xff, 0xff, 0xab, 0x9e, 0xf9, 0xff, 0x1f, 0x44, 0x03, 0xf9, 0x27, 0x89,
  0xd6, 0x0b, 0x02, 0xd3, 0xfe, 0x5b, 0x00, 0xf5, 0xf4, 0x01, 0xed, 0xfd,
  0x05, 0xf4, 0xcf, 0x24, 0x30, 0x9f, 0x1c, 0x30, 0xeb, 0xbf, 0x0c, 0x59,
  0xfa, 0x08, 0x00, 0xa0, 0x8f, 0x18, 0xfe, 0xd5, 0x43, 0xf0, 0x03, 0x7e,
  0x08, 0x3e, 0xc0, 0x0e, 0xc4, 0x40, 0xfa, 0x21, 0x50, 0xee, 0x7f, 0x3f,
  0x10, 0x03, 0xf9, 0xff, 0x98, 0x21, 0x18, 0xc4, 0x06, 0xe3, 0xfb, 0x10,
  0xbd, 0x00, 0xcf, 0x10, 0xed, 0xa2, 0x7e, 0x04, 0x00, 0x00
};

#define favicon_ico_gz_len 418


const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href='https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css' crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
     background-color: lightgray;
    }
    input[type=range] { background-color: lightgray; width: min(50vw, 220px); }
    select:disabled { -webkit-text-fill-color: black; }
    h2 { font-size: 2.0rem; }
    h4 {margin-block: 1.4em; background: rgb(197, 197, 197); border-radius: 8px; margin-inline: 20px;}
    summary {background: rgb(197, 197, 197); border-radius: 8px; margin-inline: 20px; font-weight: bold; margin-block: 0.5em;}
    h5 {margin-block: 1.3em;}
    p { font-size: 1.4rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.3rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
    .slider {position: absolute; top: 0; left: 0; right: 0; bottom: 0; background-color: #888; border-radius: 34px}
    .slider:before {position: absolute; content: ""; height: 14px; width: 14px; left: 4px; bottom: 3px; background-color: #fff; -webkit-transition: .4s; transition: .4s; border-radius: 68px}
    input:checked+.slider {background-color: #2196F3}
    input:checked+.slider:before {-webkit-transform: translateX(18px); -ms-transform: translateX(18px); transform: translateX(18px)}
  </style>
</head>
<body style="width: min(90vw, 350px); margin-left: auto; margin-right:auto;">
  <h3>Greenkea</h3>
  <p>
    <i class="fa fa-thermometer-half" style="color:#059e8a;"></i> 
    <span id="temperature">%TEMPERATURE%</span><sup class="units">&deg;C</sup>&nbsp;
    <i class="fa fa-tint" style="color:#00add6;"></i> 
    <span id="humidity">%HUMIDITY%</span><sup class="units">&percnt;</sup>
  </p>
  <p>
  <i class="fa fa-refresh" aria-hidden="true" style="color:#444444;" onclick="updateSensorsSliders(this)"></i>
  &nbsp;&nbsp;&nbsp;
  <i class="fa fa-unlock" aria-hidden="true" onclick="checklock(this)" id="lockid"></i>
  </p>
  <h4>Lights &nbsp;
  <i class="fa fa-floppy-o clickdis" aria-hidden="true" onclick="callUrl(this, '/saveall')"></i> &nbsp;
  <i class="fa fa-lightbulb-o clickdis" aria-hidden="true" style="font-size: 15pt;color:%LIGHTDIM%;" onclick="callUrl(this, '/dimonoff')"></i> &nbsp;
  <i class="fa fa-clock-o clickdis" aria-hidden="true" style="font-size: 15pt;color:%LSTARTEND%;" onclick="callUrl(this, '/starendonoff')"></i> 
  </h4>
  %LIGHTSLIDERSPH%
  <h5>mode: %DROPDOWNPLACEHOLDER%</h5>
  <h4>Fan &nbsp;
  <i class="fa fa-volume-down clickdis" aria-hidden="true" style="font-size: 15pt;color:%FANSILSTATE%;" onclick="callUrl(this, '/silfan')"></i></h4>
  %FANSLIDERSPH%
  %FANTEMPOFFSET%
  <h5>mode: %DROPDOWNFANPLACEHOLDER%</h5>
  <h4>Schedule</h4>
  %HOURONOFF%
  %WARNS%
  <details>
    <summary>Info &amp; Config</summary>
    %INFO%
    <form action="/setcfg" name="cfgformgrow" target="formtarget">
      <select id="cfgselgrow" name="cfgsel">
        <option value="minfanspeed">Min Fan Speed</option>
        <option value="maxfanspeed">Max Fan Speed</option>
        <option value="lintemposet">Linear Temp Offset</option>
        <option value="fansilmins">Fan Silent Mins</option>
        <option value="fansilspeed">Fan Silent Speed</option>
        <option value="startdimtemp">Start Dim Temp</option>
        <option value="lightofftemp">Lights Off Temp</option>
        <option value="timeonmins">Drying Time ON Mins</option>
        <option value="fanpermins">Drying Period mins</option>
        <option value="drymaxhumid">Drying Max Humidity</option>
      </select>
      <input type="text" name="cfgval" style="width: 90px;">
      <input type="reset" value="Set" onclick="document.forms['cfgformgrow'].submit();">
    </form>
    <details>
      <summary>Connections' Configs</summary>
      %INFOCONN%
      <form action="/setcfg" name="cfgform" target="formtarget">
        <select id="cfgsel" name="cfgsel">
          <option value="cmd">cmd</option>
          <option value="wifissid">WiFi SSID</option>
          <option value="wifipass">WiFi Pass</option>
          <option value="wifiip">WiFi IP</option>
          <option value="wifigw">WiFi Gateway</option>
          <option value="logcloudflag">Enable Log</option>
          <option value="logcloudurl">Log URL</option>
          <option value="logcloudapikey">Log API key</option>
          <option value="logperiodmins">Log Period</option>
        </select>
        <input type="text" name="cfgval">
        <input type="reset" value="Set" onclick="document.forms['cfgform'].submit();">
      </form>
    </details>
    <iframe name="formtarget" id="formtarget" style="height: 21pt;"></iframe>
    <br>
    <i class="fa fa-power-off" aria-hidden="true" style="font-size: 20pt;color:#444444;" onclick="buttonRestart(this)"></i>
  </details>
  <h6 style="margin-block:1.33em;">%DATETIME%</h6>
</body>
<script>
function onslideSliderPWM(element, slider_id) {
  var sliderValue = element.value;
  var textSlider = document.getElementById("textSliderValueLight"+slider_id);
  textSlider.innerHTML = sliderValue;
}

function updateSliderPWM(element, slider_id) {
  var sliderValue = element.value;
  var textSlider = document.getElementById("textSliderValueLight"+slider_id);
  textSlider.innerHTML = sliderValue;
  textSlider.style.color = "red";
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      textSlider.style.color = null;
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
      if(elem != undefined){elem.style.color = null;}
    }
  };
  xhttp.open("GET", "/getsenslivals", true);
  xhttp.send();
  if(elem != undefined){elem.style.color = "red";}

}
function buttonRestart(elem) {
  if (confirm("/!\ Gonna Reboot!!! :O")) {
    var prev_color = elem.style.color;
    elem.style.color = "red";
    var xhr = new XMLHttpRequest();
    xhr.onreadystatechange = function() {
      if (this.readyState == 4 && this.status == 200) {
        console.log("restarted");
        elem.style.color = "green";
        var xhral = new XMLHttpRequest();
        xhral.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
            elem.style.color = prev_color;
            clearInterval(tmr);
          }
        };
        var tmr = setInterval(function ( ) {
          xhral.open("GET", "/alive", true);
          xhral.send();
        }, 1000);
      }
    };
    xhr.open("GET", "/restartesp", true);
    xhr.send();
  }
}
function checklock(elem) {
  var inputs = Array.from(document.getElementsByTagName('input'))
            .concat(Array.from(document.getElementsByTagName('select') ))
            .concat(Array.from(document.getElementsByClassName('clickdis') ));
  var next_state = true;
  if(elem.classList.contains("fa-lock")){
    next_state = false;
    elem.classList.remove("fa-lock");
    elem.classList.add("fa-unlock");
  } else {
    elem.classList.remove("fa-unlock");
    elem.classList.add("fa-lock");
  }
  for (var i = 0; i < inputs.length; i++) {
    inputs[i].disabled = next_state;
  }
}
checklock(document.getElementById("lockid"));
function callUrl(element, url) {
  if (element.classList.contains("clickdis") 
  && document.getElementById("lockid").classList.contains("fa-lock") ) { return false; }
  element.style.color = "red";
  var xhr = new XMLHttpRequest();
  xhr.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      element.style.color = null;
    }
  };
  xhr.open("GET", url, true);
  xhr.send();
}
console.log("done setuping");
</script>
</html>
)rawliteral";

// Replaces placeholder with DHT values
String processor(const String& var){
  //Serial.println("processor "+var);
  if(var == "TEMPERATURE"){
    // float t = NAN;
    // readDHTTemperatureHumidity(&t, &h);
    // sprintf(slidarValsChar, "%.2f,%.2f", t,h);
    return String(readDHTTemperature(), 1);
  }
  else if(var == "HUMIDITY"){
    return String(readDHTHumidity(), 1);
  } else if (var == "LIGHTSLIDERSPH"){
    String buttons ="";
    pwmValsInfo sliderInfo = getPwmVals();
    for(int i=0; i < NLIGHTS; i++){
      String sliderVal = String(sliderInfo.vals[i]);
      buttons+= "<h5>" + String(sliderInfo.pwmNames[i]) + ": <input type=\"range\" onchange=\"updateSliderPWM(this, '" + String(i) +
      "')\" oninput=\"onslideSliderPWM(this, '" + String(i) + "')\" id=\"pwmSlider" + String(i) +"\" min=\"0\" max=\"255\" value=\""+ sliderVal + 
      "\" step=\"15\" class=\"sliderlight\"><span id=\"textSliderValueLight"+ String(i) +"\">"+sliderVal+"</span></h5>";
    }
    return buttons;
  } else if (var == "FANSLIDERSPH"){
    String buttons ="";
    pwmValsInfo sliderInfo = getPwmVals();
    for(int i=NLIGHTS; i<=NLIGHTS; i++){// sliderInfo.lenght; i++){ // only a single fan controled is supported!
      String sliderVal = String(sliderInfo.vals[i]);
      buttons+= "<h5>" + String(sliderInfo.pwmNames[i]) + ": <input type=\"range\" onchange=\"updateSliderPWM(this, '" + String(i) +
      "')\" oninput=\"onslideSliderPWM(this, '" + String(i) + "')\" id=\"pwmSlider" + String(i) +"\" min=\"0\" max=\"255\" value=\""+ sliderVal + 
      "\" step=\"5\" class=\"sliderlight\"><span id=\"textSliderValueLight"+ String(i) +"\">"+sliderVal+"</span></h5>";
    }
    return buttons;
  } else if (var == "DATETIME"){
    return getDateTime(false);
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
    String main_fan_goal_temp_str = String(main_fan_goal_temp);
    String text = "<h5>T: <input type=\"range\" onchange=\"updateSliderPWM(this, '" + temp_offset_id +
      "')\" oninput=\"onslideSliderPWM(this, '" + temp_offset_id + "')\" id=\"pwmSlider" + temp_offset_id +"\" min=\""+String(min_fan_goal_temp)+"\" max=\""+String(max_temp_lights)+"\" value=\""+ main_fan_goal_temp_str + 
      "\" step=\"1\" class=\"sliderlight\"><span id=\"textSliderValueLight"+ temp_offset_id +"\">"+main_fan_goal_temp_str+"</span>&deg;C</h5>";
     // hours off
    return text;
  } else if(var == "WARNS") {
    String text = "<span style=\"color:red\">";
    if(n_times_temp_nan > 2)
      text += "temp is nan 3x!<br>";
    text += "</span>";
    return text;
  } else if(var == "INFO") {
    int start_dim_temp_C = getStartDimTempInC();
    String text = "Lights > Off "+String(max_temp_lights)+"&deg;C"+" | Dim "+String(start_dim_temp_C)+"&deg;C (v "+String(start_dim_temp)+") | r "+String(dim_ratio)+
      //"<br>Light S2 > "+String(lights_s2_pwms[NLIGHTS-1])+" | "+String(state_s2_len_mins)+" mins"+
      "<br>Fan > "+String(min_fan_speed)+"-"+String(max_fan_speed)+" | LTO "+String(linear_temp_offset)+"&deg; | Sil "+String(mins_silent)+"m @ "+String(silent_fan_speed)+
      "<br>Drying > P "+String(fan_period_mins)+"m | "+String(time_on_mins)+"m ON"+" | "+String(dry_max_humid)+"&percnt; max H"+
      "<br>"+server_started_at+
      "<br>";
    return text;
  } else if(var == "INFOCONN") {
    String text = "WiFi: "+ssid+" | "+password+
      "<br>Log to Cloud: " + String(logToCloudFlag) + " @ " + serverName + " (key: "+ apiKeyValue +") | "+String(log_period_mins)+" P mins"+"<br>";
    return text;
  }else if(var == "FANSILSTATE") {
    String text = off_color;
    if(fan_sil_mode_is_on){
      text = on_color;
    }
    return text;
  } else if(var == "LIGHTDIM") {
    String text = off_color;
    if(getLightStageDim()){
      text = on_color;
    }
    return text;
  } else if(var == "LSTARTEND") {
    String text = off_color;
    if(getLightStageCheckStartEnd()){
      text = on_color;
    }
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
  // Route for root / web page
  server.on("/alive", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "OK");
  });
  
  // Send a the value of the sensors and light /getsenslivals
  server.on("/getsenslivals", HTTP_GET, [] (AsyncWebServerRequest *request) {
    float t = NAN;
    float h = NAN;
    readDHTTemperatureHumidity(&t, &h);
    sprintf(slidarValsChar, "%.1f,%.1f", t,h);
    
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
        setMainFanGoalTemp(sliderValueAux);  
      } else if(sliderID==NLIGHTS){//  if main fan, staps AutoFAn control
        manuallySetMainFanPwm(sliderValueAux);
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
  // SET CONFIG Send a GET request to <ESP_IP>/setcfg?cfgsel=<>&cfgval=<> 
  server.on("/setcfg", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputCfgSel, inputCfgVal;
    String msg = "OK";
    if (request->hasParam(PARAM_INPUT_CFGSEL) && request->hasParam(PARAM_INPUT_CFGVAL)) {
      inputCfgSel = request->getParam(PARAM_INPUT_CFGSEL)->value();
      inputCfgVal = request->getParam(PARAM_INPUT_CFGVAL)->value();
      bool is_int = inputCfgVal.toInt() != 0;
      int inputCfgValInt = inputCfgVal.toInt();

      msg = "Invalid Input! :(";
      if(inputCfgSel == "wifissid") {
        setWiFiSSID(inputCfgVal);
        msg = "setcfg > wifissid :" + inputCfgVal;
      } else if(inputCfgSel == "wifipass"){
        setWiFiPass(inputCfgVal);
        msg = "setcfg > wifipass :" + inputCfgVal;
      } else if (inputCfgSel == "minfanspeed"){
        if(is_int){
          setMinFanSpeed(inputCfgValInt);
          msg = "minfanspeed: "+String(inputCfgValInt);
        }
      } else if (inputCfgSel == "maxfanspeed"){
        if(is_int){
          manuallySetMaxFanSpeed(inputCfgValInt);
          msg = "maxfanspeed: "+String(inputCfgValInt);
        }
      } else if (inputCfgSel == "startdimtemp"){
        if(is_int){
          setStartDimTemp(inputCfgValInt);
          msg = "startdimtemp: "+String(inputCfgValInt);
        }
      } else if (inputCfgSel == "lightofftemp"){
        if(is_int){
          setLightsOffTemp(inputCfgValInt);
          msg = "lightofftemp: "+String(inputCfgValInt);
        }
      } else if(inputCfgSel == "lintemposet"){
        if(is_int){
          setLinearTempOffset(inputCfgValInt);
          msg = "lintemposet: "+String(inputCfgValInt);
        }
      } else if(inputCfgSel == "fansilmins"){
        if(is_int){
          setFanMinsSilent(inputCfgValInt);
          msg = "fansilmins: "+String(inputCfgValInt);
        }
      } else if(inputCfgSel == "fansilspeed"){
        if(is_int){
          setFanSilFanSpeed(inputCfgValInt);
          msg = "fansilspeed: "+String(inputCfgValInt);
        }
      } else if(inputCfgSel == "wifiip"){
        setWiFiIP(inputCfgVal);
        msg = "setcfg > wifiIP :" + inputCfgVal;
      } else if(inputCfgSel == "wifigw"){
        setWiFigateway(inputCfgVal);
        msg = "setcfg > wifigateway: " + inputCfgVal;
      } else if(inputCfgSel == "timeonmins"){
        if(is_int){
          setTimeOnMins(inputCfgValInt);
          msg = "perctimeon: "+String(inputCfgValInt);
        }
      } else if(inputCfgSel == "fanpermins"){
        if(is_int){
          setFanPeriodMins(inputCfgValInt);
          msg = "fan_period_mins: "+String(inputCfgValInt);
        }
      } else if(inputCfgSel == "drymaxhumid"){
        if(is_int){
          setFanDryMaxHumid(inputCfgValInt);
          msg = "drymaxhumid: "+String(inputCfgValInt);
        }
      } else 
      // if(inputCfgSel == "lightS2lmins"){
      //   if(is_int){
      //     setState2LenMins(inputCfgValInt);
      //     msg = "lightState2_len_mins: "+String(inputCfgValInt);
      //   }
      // } else 
      // if(inputCfgSel == "lightS2redv"){
      //   if(is_int){
      //     setState2RedVal(inputCfgValInt);
      //     msg = "lightState2_red_val: "+String(inputCfgValInt);
      //   }
      // } else 
      if (inputCfgSel == "logcloudflag"){
        if(is_int){
          setLogCloudFlag(inputCfgValInt);
          msg = "logcloudflag: "+String(inputCfgValInt);
        }
      } else if (inputCfgSel == "logcloudurl"){
        setLogServerURL(inputCfgVal);
        msg = "logcloudurl: " + inputCfgVal;
      } else if (inputCfgSel == "logcloudapikey"){
        setLogServerAPIkey(inputCfgVal);
        msg = "logcloudapikey: " + inputCfgVal;
      } else if (inputCfgSel == "logperiodmins"){
        if(is_int){
          setLogPeriodMins(inputCfgValInt);
          msg = "logperiodmins: "+String(inputCfgValInt);
        }
      // CMD START
      } else if(inputCfgSel == "cmd"){
        msg = "cmd > unknown cmd! :( " + inputCfgVal;
        if(inputCfgVal.length() == 10 && inputCfgVal == "resetprefs"){
          clearPrefs();
          msg = "cmd resetprefs > done :)";
        } 
        else if (inputCfgVal.length() > 8 && inputCfgVal.substring(0,8) == "resetkey") {
          bool ret = clearKey(inputCfgVal.substring(9));
          msg = "cmd resetkey > key: "+inputCfgVal.substring(9)+"  ret: "+String(ret)+"  done :)";
        }
        else if (inputCfgVal.length() > 4 && inputCfgVal.substring(0,4) == "setl") {
          //const char * input = inputCfgVal.substring(9,20).c_str();
          char input[NLIGHTS*4+20];
          inputCfgVal.substring(5).toCharArray(input, NLIGHTS*4+20);
          char* sub_command = strtok(input, " ");
          char* val = strtok(NULL, " ");
          int n_vals = 0;
          int vals_arr_new[NLIGHTS];
          fillArray(vals_arr_new, NLIGHTS, 1);
          String vals_str = String("");
          while (val != NULL)
          { 
            //Serial.println("sdp while > " + String(n_vals)+" "+String(val));
            if(n_vals == NLIGHTS) break;
            vals_arr_new[n_vals] = atoi(val);
            n_vals++;
            val = strtok(NULL, " ");
          }
          msg = "cmd setl "+String(sub_command)+" > done :)";
          String sub_cmd_str = String(sub_command);
          if(sub_cmd_str == "dprio")
            setDimPrio(vals_arr_new);
          else if(sub_cmd_str == "startdel")
            setStartDelayArr(vals_arr_new);
          else if(sub_cmd_str == "endearly")
            setEndEarlyArr(vals_arr_new);
          else if(sub_cmd_str == "startend"){
            setStartDelayArr(vals_arr_new);
            setEndEarlyArr(vals_arr_new);
          } else
            msg = "cmd setl '"+String(sub_command)+"' > unknown :(";
          
        } else if(inputCfgVal.length() > 5 && inputCfgVal.substring(0,5) == "infol"){
          String sub_command = inputCfgVal.substring(6);
          int * into_arr_ptr = NULL;
          if(sub_command == "dprio")
            into_arr_ptr = dim_priority_arr;
          else if(sub_command == "startdel")
            into_arr_ptr = lights_start_delay_mins_arr;
          else if(sub_command == "endearly")
            into_arr_ptr = lights_end_early_mins_arr;
          String vals_str = String("");
          if (into_arr_ptr != NULL) {
            for(int i=0; i<NLIGHTS;i++){
              vals_str += String(into_arr_ptr[i])+" ";
            }
            msg = sub_command+" > "+vals_str;
          }
        } 
      // CMD END
      } else
        msg = "setcfg > unknown config: " + inputCfgVal;
    }
    else {
      Serial.println("setcfg: not enought variables!");
    }
    Serial.println(msg);
    request->send(200, "text/plain", msg.c_str());
  });
  // save all
  server.on("/saveall", HTTP_GET, [](AsyncWebServerRequest *request){
    saveLightModes();
    request->send(200, "text/plain", "OK");
  });
  // silfan
  server.on("/silfan", HTTP_GET, [](AsyncWebServerRequest *request){
    startFanSilentMode();
    request->send(200, "text/plain", "OK");
  });
  // dimonoff
  server.on("/dimonoff", HTTP_GET, [](AsyncWebServerRequest *request){
    switchLightStageDim();
    request->send(200, "text/plain", "OK");
  });
  // starendonoff
  server.on("/starendonoff", HTTP_GET, [](AsyncWebServerRequest *request){
    switchLightStageCheckStartEnd();
    request->send(200, "text/plain", "OK");
  });
  // favicon
  // https://github.com/me-no-dev/ESPAsyncWebServer#send-binary-content-from-progmem
  // Convert to favicon and gzip it:  https://www.favicon-generator.org/
  // convert to binary via terminal: xxd -i favicon.ico.gz
  server.on("/favicon.ico", HTTP_GET, [](AsyncWebServerRequest *request){
    //Serial.println("favicon.ico");

    AsyncWebServerResponse *response = request->beginResponse_P(200, "image/x-icon", favicon_ico_gz, favicon_ico_gz_len);
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
  });
  // Start server
  server.begin();
  server_started_at = getDateTime(false);
}
