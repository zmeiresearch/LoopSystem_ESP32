function updateConfig(data)
{
    console.log("Updating config values");
    
    $('#wifiSsid').val(data["wifi"]["ssid"]);
    $('#wifiPassword').val(data["wifi"]["password"]);
    $('#buildId').text(data["system"]["buildId"]);
    $('#buildTime').text(data["system"]["buildTime"]);        
}

async function set_config()
{
    wifi = {
        'ssid' : $('#wifiSsid').val(),
        'password' : $('#wifiPassword').val(),
    };

    $.ajax("config", {
        data : JSON.stringify({"wifi" : wifi}),
        contentType : 'application/json',
        type : 'POST'
        }).always(function(done) {
            setTimeout(get_config, 600);
        });
}

document.addEventListener('DOMContentLoaded', function() {
    console.log("system.js: Registering status handler");
    if (!window.socketEventHandlers) window.socketEventHandlers = {};
    window.socketEventHandlers["Config"] = updateConfig;
    requestConfig();
}, false);
