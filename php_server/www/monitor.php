<?php

session_start();
if ( ! isset($_SESSION['loggedIn']) || $_SESSION['loggedIn'] == 0) {
    // not logged in
    header('Location: login.php');
    exit;
}

$servername = $_ENV['MYSQL_SERVERNAME'];
$dbname = $_ENV['MYSQL_DATABASE'];
$username = $_ENV['MYSQL_USER'];
$password = $_ENV['MYSQL_PASSWORD'];

// Create connection
$conn = new mysqli($servername, $username, $password, $dbname);
// Check connection
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
} 

$sql = "SELECT datetime, temp, humd, fanspeed, avglight FROM sensors_values ORDER BY datetime DESC";

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
    <br>
    <div id="light_plot"></div>
    <br>
    <div id="table_div"></div>
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
    styles = 'a.modebar-btn {font-size: 45px !important;height: 45px !important;}';
} else {
    styles = 'a.modebar-btn {font-size: 20px !important;height: 26px !important;}';
}
var styleSheet = document.createElement("style");
styleSheet.innerText = styles;
document.head.appendChild(styleSheet);

// from: https://blog.logrocket.com/top-picks-javascript-chart-libraries/
// from: https://github.com/plotly/plotly.js

function newScatterPlot(data_col, color_val, title_val, yaxis_title, ticksuffix_val, div_id, add_extras) {

    var config_plot = {responsive: true};

    var data_plot = [
      {
        x: data_sensors['datetime'],
        y: data_sensors[data_col],
        type: 'scatter',
        marker: {
            color: color_val,
            size: 8
        },
        line: {
            color: color_val,
            width: 3
        }
      }
    ];
    const layout = {
        title: title_val,
        height: 320,
        margin: {
            l: 80,
            r: 50,
            b: 70,
            t: 60,
            pad: 4
        },
        xaxis: {
            //autorange: true,
            automargin: true,
            range: [data_sensors['datetime'][70], data_sensors['datetime'][0]],
            //rangeslider: {range: [data_sensors['datetime'].slice(-1)[0], data_sensors['datetime'][0]]},
            type: 'date',
            nticks: 26,
            //tickformat: '%d %b',
        },
        yaxis: {
          autorange: true,
          title: yaxis_title,
          ticksuffix: ticksuffix_val
        },
        modebar: {
          orientation: 'v',
          remove:['autoscale','lasso', 'select', 'zoomin', 'zoomout'],
        },
    };
    if (isMobile){
        data_plot[0].marker.size = 9;
        data_plot[0].line.width = 5;
        layout.font = {size: 28};
        layout.margin = {
            l: 120,
            r: 50,
            b: 120,
            t: 120,
            pad: 4
        };
        layout.height = 480;
        layout.xaxis.nticks = 13;
    }
    if(title_val == ''){
        delete layout['title'];
        layout.height = layout.height-layout.margin.t;
        layout.margin.t = 5;
    }

    if(add_extras){
        layout.xaxis.rangeselector = {
            buttons: [
                {
                  count: 8,
                  label: '8h',
                  step: 'hour',
                  stepmode: 'backward'
                },
                {
                  count: 1,
                  label: '1d',
                  step: 'day',
                  stepmode: 'backward'
                },
                {
                  count: 3,
                  label: '3d',
                  step: 'day',
                  stepmode: 'backward'
                },
                {
                  count: 7,
                  label: '7d',
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
              ]
          };


    }
    Plotly.newPlot(div_id, data_plot, layout, config_plot);
}

newScatterPlot('temp', 'orange', 'Temp & Humid', 'Temperature','°C', 'temp_plot', true);
newScatterPlot('humd', 'blue', '', 'Humidity','%', 'humd_plot', false);
newScatterPlot('fanspeed', 'green', 'Fan Speed', '0-255','', 'fans_plot', true);
newScatterPlot('avglight', 'red', 'Avg Light', '0-255','', 'light_plot', false);

var col_names = []; // [["<b>EXPENSES</b>"], 
var data_values = [];
Object.entries(data_sensors).forEach(([key, value]) => {
    col_names.push([key]);
    data_values.push(value);
});

//data_values_T = data_values[0].map((_, colIndex) => data_values.map(row => row[colIndex]));

var datetime_arr = data_sensors['datetime'];
var cells_fill = ['white'];
for (let i = 0; i < datetime_arr.length-1; i++) {
    var time_diff = (new Date(datetime_arr[i])-new Date(datetime_arr[i+1]))/(1000*60); // ms to mins
    //console.log(time_diff, datetime_arr[i], datetime_arr[i+1])
    if (time_diff < 25 ) {
        cells_fill.push('white');
    } else {
        cells_fill.push('lightyellow');
    }
}

var data_tb = [{
  type: 'table',
  header: {
    values: col_names,
    align: "center",
    //line: {width: 1, color: 'black'},
    fill: {color: "grey"},
    //font: {family: "Arial", size: 12, color: "white"}
  },
  cells: {
    values: data_values,
    align: "center",
    //line: {color: "black", width: 1},
    //font: {family: "Arial", size: 11, color: ["black"]}
    fill: {
        color: [cells_fill,'white', 'white','white', 'white']
    },
  }
}];

Plotly.newPlot('table_div', data_tb);


    // var config_plot = {responsive: true};

    // var data_plots = [
    //   {
    //     x: data_sensors['datetime'],
    //     y: data_sensors['temp'],
    //     name: 'Temperature',
    //     type: 'scatter',
    //     marker: {
    //         color: 'orange',
    //         size: 5,
    //     },
    //     line: {
    //         color: 'orange',
    //         width: 3
    //     }
    //   },
    //   {
    //     x: data_sensors['datetime'],
    //     y: data_sensors['humd'],
    //     name: 'Humidity',
    //     xaxis: 'x2',
    //     yaxis: 'y2',
    //     type: 'scatter',
    //     marker: {
    //         color: 'blue',
    //         size: 5,
    //     },
    //     line: {
    //         color: 'blue',
    //         width: 3
    //     }
    //   }
    // ];
    // const layout = {
    //     title: 'plots',
    //     grid: {
    //         rows: 2,
    //         columns: 1,
    //         pattern: 'independent',
    //     },
    //     height: 600,
    //     xaxis: {
    //         //autorange: true,
    //         range: [data_sensors['datetime'][10], data_sensors['datetime'][0]],
    //         rangeselector: {buttons: [
    //             {
    //               count: 1,
    //               label: '1d',
    //               step: 'day',
    //               stepmode: 'backward'
    //             },
    //             {
    //               count: 5,
    //               label: '5d',
    //               step: 'day',
    //               stepmode: 'backward'
    //             },
    //             {
    //               count: 1,
    //               label: '1M',
    //               step: 'month',
    //               stepmode: 'backward'
    //             },
    //             {step: 'all'}
    //           ]},
    //         //rangeslider: {range: [data_sensors['datetime'].slice(-1)[0], data_sensors['datetime'][0]]},
    //         type: 'date',
    //         nticks: 26,

    //     },
    //     xaxis2: {
    //         range: [data_sensors['datetime'][10], data_sensors['datetime'][0]],
    //         type: 'date',
    //         nticks: 26,

    //     },
    //     yaxis: {
    //       autorange: true,
    //       title: 'Temperature',
    //       ticksuffix: '°C'
    //     },
    //     yaxis2: {
    //       autorange: true,
    //       title: 'Humidity',
    //       ticksuffix: '%'
    //     },
    //     modebar: {
    //       orientation: 'v',
    //     },
    //     showlegend: false,
    // };
    // if (isMobile){
    //     for (var i = 0; i < data_plot.length; i++) {
    //         data_plot[i].marker.size = 7;
    //         data_plot[i].line.width = 5;
    //     }
    //     layout.font = {size: 28};
    // }
    // Plotly.newPlot('temp_plot', data_plots, layout, config_plot);

// // https://stackoverflow.com/questions/47933524/how-do-i-synchronize-the-zoom-level-of-multiple-charts-with-plotly-js
var myDiv = document.getElementById("temp_plot");
var myDiv2 = document.getElementById("humd_plot");
var myDiv3 = document.getElementById("fans_plot");
var myDiv4 = document.getElementById("light_plot");

var divs = [myDiv, myDiv2, myDiv3, myDiv4];

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
      //'yaxis.autorange': ed["yaxis.autorange"],
      //'yaxis.autorange': true,
     };
     Plotly.relayout(div, update);
    }
  });
}

var plots = [myDiv, myDiv2, myDiv3, myDiv4];
plots.forEach(div => {
  div.on("plotly_relayout", function(ed) {
    relayout(ed, divs);
  });
}); 
console.log('done');
</script>
</body>
</html>