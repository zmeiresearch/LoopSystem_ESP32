function receiveSystemStatus(data)
{
    console.log("Updating status values");
    
    $('#wifiSsid').text(data["wifi"]["ssid"]);
    $('#wifiStatus').text(data["wifi"]["status"]);
    $('#buildId').text(data["system"]["buildId"]);
    $('#buildTime').text(data["system"]["buildTime"]);
    $('#memoryStats').text(data["system"]["memoryStats"]);
}

document.addEventListener('DOMContentLoaded', function() {
    console.log("system.js: Registering status handler");
    if (!window.socketMessageHandlers) window.socketMessageHandlers = {};
    window.socketMessageHandlers["SystemStatus"] = receiveSystemStatus;
    requestSystemStatus();
}, false);
