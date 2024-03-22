<?php

$tempFilePath = $_FILES["file"]["tmp_name"];
$myFile = fopen("../Published_Data/update.txt", "w");

$homepage = file_get_contents($tempFilePath);
file_put_contents("../file.bin",$homepage);
header("Location: ../index.html");
fwrite($myFile, '1');
fclose($myFile);
?>

