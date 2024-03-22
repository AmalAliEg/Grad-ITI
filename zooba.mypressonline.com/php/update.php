<?php
// Specify the path to the file you want to read
/*$file_path = '../Published_Data/distance.txt';

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    // Get the POST data
    $data = isset($_POST['data']) ? $_POST['data'] : '';
        
        $data = file_get_contents('php://input');
    file_put_contents('distance.txt', $data);

    // Respond with a success message
    echo 'Data received and stored successfully.';
} else {
    // Respond with an error message for non-POST requests
    echo 'Invalid request method.';
}*/


// Check if data is sent via POST
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    // Retrieve data from the POST request
    $data = $_POST['data'];

    // File to write data to
    $file = '../Published_Data/update.txt';

    // Open the file in write mode, which truncates the file
    $fileHandle = fopen($file, 'w');

    // Write data to the file
    fwrite($fileHandle, $data);

    // Close the file
    fclose($fileHandle);

    // Respond with success message
    echo "Data written successfully.";
} else {
    // Respond with error message if not a POST request
    http_response_code(405);
    echo "Method Not Allowed";
}


?>