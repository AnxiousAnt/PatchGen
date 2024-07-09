<?php
$fp = fsockopen("udp://127.0.0.1", 13000, $errno, $errstr, 50);
if (!$fp) {
    echo "Fehler: $errno - $errstr<br>\n";
} else {
    fwrite($fp,"\n");
    echo fread($fp, 26);
 //   fclose($fp);
}
?>