<?php
// https://stackoverflow.com/questions/35128314/full-password-protected-website
session_start();
if ( isset($_SESSION['loggedIn']) && $_SESSION['loggedIn'] == 1) {
    // already logged in
    header('Location: monitor.php');
    exit;
}
if ($_SERVER["REQUEST_METHOD"] == "POST") {
    If ( $_POST["pass"] == "123"){
        $_SESSION['loggedIn'] = 1;
        header('Location: monitor.php');
        exit;
    } else {
        unset($_SESSION['loggedIn']);
        header('Location: login.php');
        exit;
    }
}

?> 
<!DOCTYPE html>
<html lang="en">
<head>
   <meta charset="UTF-8">
   <title>Login</title>
</head>
<body>
<form role="form" method="POST" style="margin: 0px" action="login.php">
   <input type="password" name ="pass" class="form-control" id="pwd" />  
   <input type="submit" name="login" class="btn btn-danger" value="Login" />
</form>
</body>
</html>