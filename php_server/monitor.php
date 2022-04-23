<?php

session_start();
if ( ! isset($_SESSION['loggedIn']) || $_SESSION['loggedIn'] == 0) {
    // not logged in
    header('Location: login.php');
    exit;
}

$servername = "localhost";
$dbname = "id17187452_esp32ghs";
$username = "id17187452_esp32ghsun";
$password = "";

// Create connection
$conn = new mysqli($servername, $username, $password, $dbname);
// Check connection
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
} 

$sql = "SELECT datetime, temp, humd, fanspeed FROM sensorvals ORDER BY datetime DESC";

$cols_array_dict = array();
if ($result = $conn->query($sql)) {
    // get names of columns
    while ($row = $result->fetch_field()) {
        $cols_array_dict[$row->name] = array();
    }
    // populate column arrays
    while ($row = $result->fetch_assoc()) { //  fetch_assoc fetch_array fetch_row
        foreach($cols_array_dict as $key => $vals){
            $cols_array_dict[$key][] = $row[$key];
        }
    }
    $result->free();
}
$conn->close();

?> 
<!DOCTYPE html>
<html>
<head>
    <script src="https://cdn.plot.ly/plotly-2.11.1.min.js"></script>
</head>
<body>
<div style="width: 80%; margin-left: 10%;">
<div id="temp_plot"></div>
<br>
<div id="humd_plot"></div>
<br>
<div id="fans_plot"></div>
</div>
<script>
  
// Access the array elements
var data_sensors = 
    <?php echo json_encode($cols_array_dict); ?>;

var isMobile = false;
if( /Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(navigator.userAgent) ) {
 isMobile = true;
}

var styles;
if (isMobile){
    styles = 'a.modebar-btn {font-size: 35px !important;}';
} else {
    styles = 'a.modebar-btn {font-size: 20px !important;}';
}
var styleSheet = document.createElement("style");
styleSheet.innerText = styles;
document.head.appendChild(styleSheet);

// from: https://blog.logrocket.com/top-picks-javascript-chart-libraries/
// from: https://github.com/plotly/plotly.js

function newScatterPlot(data_col, color_val, title_val, yaxis_title, ticksuffix_val, div_id) {

    var config_plot = {responsive: true};

    var data_plot = [
      {
        x: data_sensors['datetime'],
        y: data_sensors[data_col],
        type: 'scatter',
        marker: {
            color: color_val,
            size: 5
        },
        line: {
            color: color_val,
            width: 3
        }
      }
    ];
    const layout = {
        title: title_val,
        xaxis: {
            //autorange: true,
            range: [data_sensors['datetime'][10], data_sensors['datetime'][0]],
            rangeselector: {buttons: [
                {
                  count: 1,
                  label: '1d',
                  step: 'day',
                  stepmode: 'backward'
                },
                {
                  count: 5,
                  label: '5d',
                  step: 'day',
                  stepmode: 'backward'
                },
                {
                  count: 1,
                  label: '1M',
                  step: 'month',
                  stepmode: 'backward'
                },
                {step: 'all'}
              ]},
            //rangeslider: {range: [data_sensors['datetime'].slice(-1)[0], data_sensors['datetime'][0]]},
            type: 'date',
        },
        yaxis: {
          autorange: true,
          title: yaxis_title,
          ticksuffix: ticksuffix_val
        },
        modebar: {
          orientation: 'v',
        },
    };
    if (isMobile){
        data_plot[0].marker.size = 7;
        data_plot[0].line.width = 5;
        layout.font = {size: 28};
    }
    Plotly.newPlot(div_id, data_plot, layout, config_plot);
}

newScatterPlot('temp', 'orange', 'Temperature', '','°C', 'temp_plot');
newScatterPlot('humd', 'blue', 'Humidity', '','%', 'humd_plot');
newScatterPlot('fanspeed', 'green', 'Fan Speed', '0-255','', 'fans_plot');

// https://stackoverflow.com/questions/47933524/how-do-i-synchronize-the-zoom-level-of-multiple-charts-with-plotly-js
var myDiv = document.getElementById("temp_plot");
var myDiv2 = document.getElementById("humd_plot");
var myDiv3 = document.getElementById("fans_plot");

var divs = [myDiv, myDiv2, myDiv3];

function relayout(ed, divs) {
  if (Object.entries(ed).length === 0) {return;}
  divs.forEach((div, i) => {
    let x = div.layout.xaxis;
    if (ed["xaxis.autorange"] && x.autorange) return;
    if (x.range[0] != ed["xaxis.range[0]"] ||x.range[1] != ed["xaxis.range[1]"])
    {
      var update = {
      'xaxis.range[0]': ed["xaxis.range[0]"],
      'xaxis.range[1]': ed["xaxis.range[1]"],
      'xaxis.autorange': ed["xaxis.autorange"],
     };
     Plotly.relayout(div, update);
    }
  });
}

var plots = [myDiv, myDiv2, myDiv3];
plots.forEach(div => {
  div.on("plotly_relayout", function(ed) {
    relayout(ed, divs);
  });
}); 
</script>
</body>
</html>