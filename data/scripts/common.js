function sleep(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
}

function format_for_display(val) {
    return (val / 10).toFixed(1);
}

function format_from_display(val) {
    return parseInt((val * 10).toFixed(0));
}

function setElementValueValid(e) {
    e.classList.remove("value_invalid");
    e.classList.add("value_valid");
}

function setElementValueInvalid(e) {
    e.classList.remove("value_valid");
    e.classList.add("value_invalid");
}

function setSaveButtonEnabled(e) {
    e.classList.remove("save_button_disabled");
    e.classList.add("save_button_enabled");
}

function setSaveButtonDisabled(e) {
    e = document.getElementById("set_button");
    e.classList.remove("save_button_enabled");
    e.classList.add("save_button_disabled");
}

function connectSocket() {
    socket = new WebSocket("ws://" + location.hostname + "/ws");
    socket.onopen = function(event) {
        console.log("dataSocket: opened " + event);
        //socket.send(JSON.stringify({'action':'getConfig'}));
    };

    socket.onmessage = function(event) {
        console.log("dataSocket: Got: " + event.data);

        let response = JSON.parse(event.data);
        if (window.socketMessageHandlers) {
            if (response["type"] in window.socketMessageHandlers) {
                socketMessageHandlers[response["type"]](response["data"]);
            }
        }
    };

    socket.onclose = function(event) {
        console.log("dataSocket: Closed!");
        if (window.socketCloseHandlers) {
            for (const [key, value] of Object.entries(window.socketCloseHandlers)) {
                console.log("dataSocket: calling " + value);
                value();
            } 
        }
    }

    window.dataSocket = socket;
}

async function websocketSend(data) {
    var done = false;
    while (!done) {
        if (window.dataSocket && window.dataSocket.readyState === WebSocket.OPEN) {
            window.dataSocket.send(data);
            done = true;
        }
        await sleep(100);
    }
}

async function requestConfig() {
    websocketSend(JSON.stringify({ action: "getConfig"}));
}

async function requestSystemStatus() {
    websocketSend(JSON.stringify({ action: "getSystemStatus"}));
}

async function requestModeValues(mode) {
    websocketSend(JSON.stringify({ action: "getModeValues", data: { mode: mode}}));
}

async function requestGlobalValues() {
    websocketSend(JSON.stringify({ action: "getGlobalValues"}));
}

async function sendGlobalValues(data) {
    websocketSend(JSON.stringify({ action: "setGlobalValues", data}));
}

async function sendModeValues(data) {
    websocketSend(JSON.stringify({ action: "setModeValues", data}));
}

async function sendConfig(data) {
    websocketSend(JSON.stringify({ action: "setConfig", data}));
}

document.addEventListener('DOMContentLoaded', connectSocket, false);

/*
async function get_limits(useDefaultsOnFail, callback) {
    var done = false;
    while (!done) {
        $.ajax({
            type: "GET",
            url: "/globalValues",
            success: function (data) {
                window.limits = data;
                done = true;
            },
            error: function (response) {
                if (useDefaultsOnFail) {
                    console.log("setting default limits");
                    var data = [];
                    data["home"] = 123;
                    data["maxEnd"] = 1670;
                    data["maxTurn1"] = 1520;
                    data["minTurn2"] = 235;
                    data["maxAcc"] = 32;
                    data["maxDec"] = 35;
                    data["maxSpeed"] = 300;
                    data["homingSpeed"] = 100;
                    data["maxTime"] = 3600;
                    data["maxLaps"] = 100;
                    data["servSpeed"] = 54;
                    window.limits = data;
                    done = true;
                }
            }
        });

        await sleep(1000);
    }

    if (callback) callback();
}

async function get_mode_values(useDefaultsOnFail, callback) {
    var done = false;
    while (!done) {
        var mode = $('#mode_id').text();
        $.ajax({
            type: "GET",
            data: { 'mode': mode },
            url: "/modeValues",
            success: function (data) {
                window.values = data;
                done = true;
            },
            error: function (response) {
                if (useDefaultsOnFail) {
                    console.log("setting default values");
                    var data = [];
                    data["speed"] = 134;
                    data["turn1"] = 1532;
                    data["turn2"] = 287;
                    data["brakeTime"] = 13;
                    data["acc"] = 18;
                    data["dec"] = 14;
                    window.values = data;
                    done = true;
                }
            }
        });

        await sleep(1000);
    }

    if (callback) callback();
}*/