import requests;

with open('.pio/build/esp32dev/firmware.bin', 'rb') as f:
    r = requests.post('http://192.168.1.131/updateFirmware', files={'firmware.bin': f})