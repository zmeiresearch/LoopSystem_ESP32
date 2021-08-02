import requests;

with open('.pio/build/esp32dev/spiffs.bin', 'rb') as f:
    r = requests.post('http://192.168.1.131/updateSpiffs', files={'spiffs.bin': f})