<?php

$servername = "localhost";


$dbname = "id17187452_esp32ghs";
$username = "id17187452_esp32ghsun";
$password = "g~#Do(%eVIkyM$9^";

// Keep this API Key value to be compatible with the ESP32 code provided in the project code. 
// If you change this value, the ESP32 sketch needs to match
$api_key_value = "Z2E58eFfzfBb";

$api_key= $temp_val = $humd_val = $fanspeed_val = "";

if ($_SERVER["REQUEST_METHOD"] == "POST") {
    $api_key = test_input($_POST["api_key"]);
    if($api_key == $api_key_value) {
        $date_time = test_input($_POST["datetime"]);
        $temp_val = check_numeric(test_input($_POST["temp"]));
        $humd_val = check_numeric(test_input($_POST["humid"]));
        $fanspeed_val = check_numeric(test_input($_POST["fanspeed"]));
        $avg_light_power = check_numeric(test_input($_POST["avglight"]));
        
        // Create connection
        $conn = new mysqli($servername, $username, $password, $dbname);
        // Check connection
        if ($conn->connect_error) {
            die("Connection failed: " . $conn->connect_error);
        } 
        
        $sql = "INSERT INTO sensors_values (`datetime`, `temp`, `humd`, `fanspeed`, `avglight`)
        VALUES ('" . $date_time . "', " . $temp_val . ", " . $humd_val . ", " . $fanspeed_val . ", " . $avg_light_power . ")";
        
        if ($conn->query($sql) === TRUE) {
            echo "New record created successfully";
        } 
        else {
            echo "Error: " . $sql . "<br>" . $conn->error;
        }
    
        $conn->close();
    }
    else {
        echo "Wrong API Key provided.";
    }

}
else {
    echo "No data posted with HTTP POST.";
}

function test_input($data) {
    $data = trim($data);
    $data = stripslashes($data);
    $data = htmlspecialchars($data);
    return $data;
}

function check_numeric($data) {
    if (is_numeric($data)){
        return "'" . $data . "'";
    } else {
        return "null";
    }
}