function get_config()
{
    $.get( "/config", function( data ) {
        console.log("Received: " + data);
        $('#wifiSsid').val(data["wifi"]["ssid"]);
        $('#wifiPassword').val(data["wifi"]["password"]);
        
    });
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

$(document).ready()
{
    get_config();
}
