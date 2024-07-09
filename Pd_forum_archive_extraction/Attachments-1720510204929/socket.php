<?php
#error_reporting (E_ALL);

echo "<h2>TCP/IP Connection</h2>\n";

#if($freq == "") 
$freq = rand(40,6000); 
if($dur == "") $dur = rand(200,600);


print "<form action=\"socket.php\" method=get>freq: <input type=text size=5 name=freq value=\"$freq\">, dur <input type=text size=5 name=dur value=\"$dur\"><input type=submit value=doit></form>";

/* Get the port for the WWW service. */
//$service_port = getservbyname ('www', 'tcp');
$service_port = 7007;

/* Get the IP address for the target host. */
// $address = gethostbyname ('www.php.net');
$address = "62.116.9.40";

/* Create a TCP/IP socket. */
$socket = socket_create (AF_INET, SOCK_STREAM, 0);
if ($socket < 0) {
        echo "socket_create() failed: reason: " . socket_strerror ($socket) . "\n";
} else {
        echo "OK.\n";
}

echo "Attempting to connect to '$address' on port '$service_port'...";
$result = socket_connect ($socket, $address, $service_port);
if ($result < 0) {
        echo "socket_connect() failed.\nReason: ($result) " . socket_strerror($result) . "\n";
} else {
        echo "OK.\n";
}

$in = "bla $freq $dur;";

echo "Sending data";
socket_write ($socket, $in, strlen ($in));
echo "OK.\n";

socket_close ($socket);
?>
