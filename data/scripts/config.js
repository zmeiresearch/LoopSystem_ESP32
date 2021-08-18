function receiveConfig(data)
{
    console.log("Updating config values");
    
    $('#WifiSSID').val(data["WifiSSID"]);
    $('#WifiPassword').val(data["WifiPassword"]);
    
    $('#VpnEnabled').val(data["VpnEnabled"]);
    $('#VpnLocalAddress').val(data["VpnLocalAddress"]);
    $('#VpnLocalNetmask').val(data["VpnLocalNetmask"]);
    $('#VpnClientPort').val(data["VpnClientPort"]);
    $('#VpnGateway').val(data["VpnGateway"]);
    $('#VpnClientPrivateKey').val(data["VpnClientPrivateKey"]);
    $('#VpnPeerAddress').val(data["VpnPeerAddress"]);
    $('#VpnPeerPort').val(data["VpnPeerPort"]);
    $('#VpnPeerPublicKey').val(data["VpnPeerPublicKey"]);
}

async function uploadWifiConfig()
{
    data = {
        'WifiSSID'      : $('#WifiSSID').val(),
        'WifiPassword'  : $('#WifiPassword').val(),
    };

    sendConfig(data)
}

async function uploadVpnConfig()
{
    data = {
        'VpnEnabled'            : $('#VpnEnabled').val(),
        'VpnLocalAddress'       : $('#VpnLocalAddress').val(),
        'VpnLocalNetmask'       : $('#VpnLocalNetmask').val(),
        'VpnClientPort'         : $('#VpnClientPort').val(),
        'VpnGateway'            : $('#VpnGateway').val(),
        'VpnClientPrivateKey'   : $('#VpnClientPrivateKey').val(),
        'VpnPeerAddress'        : $('#VpnPeerAddress').val(),
        'VpnPeerPort'           : $('#VpnPeerPort').val(),
        'VpnPeerPublicKey'      : $('#VpnPeerPublicKey').val(),
    };

    sendConfig(data)
}

document.addEventListener('DOMContentLoaded', function() {
    console.log("system.js: Registering status handler");
    if (!window.socketMessageHandlers) window.socketMessageHandlers = {};
    window.socketMessageHandlers["Config"] = receiveConfig;
    requestConfig();
}, false);
