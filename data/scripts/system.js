function receiveConfig(data)
{
    console.log("Updating config values");
    
    $('#wifiSsid').val(data["wifi"]["ssid"]);
    $('#wifiPassword').val(data["wifi"]["password"]);
    $('#buildId').text(data["system"]["buildId"]);
    $('#buildTime').text(data["system"]["buildTime"]);
    $('#memoryStats').text(data["system"]["memoryStats"]);
}

async function uploadConfig()
{
    data = {
        "wifi": {
            'ssid' : $('#wifiSsid').val(),
            'password' : $('#wifiPassword').val(),
    }};

    sendConfig(data)
}

document.addEventListener('DOMContentLoaded', function() {
    console.log("system.js: Registering status handler");
    if (!window.socketMessageHandlers) window.socketMessageHandlers = {};
    window.socketMessageHandlers["Config"] = receiveConfig;
    requestConfig();
}, false);
