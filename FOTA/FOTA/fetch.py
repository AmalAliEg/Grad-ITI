import urllib.request
import os
import time
import serial
import subprocess
import requests
        
def write_data_to_server():
    url = 'http://zooba.mypressonline.com/php/update.php'
    payload = {'data': '0'}  # Sending '0' as the data

    try:
        response = requests.post(url, data=payload)
        if response.status_code == 200:
            print("Data '0' written successfully to the server.")
        else:
            print("Error:", response.text)
    except requests.exceptions.RequestException as e:
        print("Error:", e)
        
def download_and_send_file(server_file_url, local_file_path):
    try:
        remote_last_modified = urllib.request.urlopen(server_file_url).info().get('Last-Modified')
        remote_last_modified = time.strptime(remote_last_modified, '%a, %d %b %Y %H:%M:%S %Z')
        remote_last_modified = time.mktime(remote_last_modified)
        
        local_last_modified = os.path.getmtime(local_file_path)
    except Exception as e:
        print(f"Error: {e}")
        return

    if remote_last_modified > local_last_modified:
        try:
            # Download the file if it has changed
            urllib.request.urlretrieve(server_file_url, local_file_path)
            CBL_Command = 1
            print("File updated. Downloaded latest version.")
            check_file_url = "http://zooba.mypressonline.com/Published_Data/update.txt"
            data_to_post = "0"
            write_data_to_server()
            subprocess.run(["python", "host.py", str(CBL_Command)])
        except Exception as e:
            print(f"Error downloading/sending file: {e}")

def check_file(check_file_url, check_file_path):
    try:
        # Get the last modified timestamp of the remote file
        remote_last_modified = urllib.request.urlopen(check_file_url).info().get('Last-Modified')
        remote_last_modified = time.strptime(remote_last_modified, '%a, %d %b %Y %H:%M:%S %Z')
        remote_last_modified = time.mktime(remote_last_modified)
        
        # Get the last modified timestamp of the local file
        local_last_modified = os.path.getmtime(check_file_path)
    except Exception as e:
        print(f"Error: {e}")
        return

    if remote_last_modified > local_last_modified:
        try:
            # Download the file if it has changed
            urllib.request.urlretrieve(check_file_url, check_file_path)

            # Read the content of the downloaded file
            with open(check_file_path, 'r') as file:
                file_content = file.read().strip()  # Assuming the file contains only the value
            
            # Compare the content with the desired value
            if file_content == '1':
                serve_file_url = "http://zooba.mypressonline.com/file.bin"
                local_file_path = "Application.bin"
                download_and_send_file(serve_file_url, local_file_path)
        except Exception as e:
            print(f"Error downloading/sending file: {e}")

# Define the URL and local path for the file
check_file_url = "http://zooba.mypressonline.com/Published_Data/update.txt"
check_file_path = "check.txt"

while True:
    try:
        check_file(check_file_url, check_file_path)
    except Exception as e:
        print(f"Error reading car status or communicating over UART: {e}")
