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
                    data["home"] = 1000;
                    data["maxEnd"] = 800000;
                    data["maxTurn1"] = 730000;
                    data["minTurn2"] = 10000;
                    data["maxAcc"] = 32;
                    data["maxDec"] = 35;
                    data["maxSpeed"] = 300;
                    data["homingSpeed"] = 100;
                    data["maxTime"] = 3600;
                    data["maxLaps"] = 100;
                    data["servSpeed"] = 50;
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
                    data["speed"] = 3000;
                    data["turn1"] = 434753;
                    data["turn2"] = 200000;
                    data["brakeTime"] = 13;
                    data["acc"] = 18;
                    data["dec"] = 14;
                    set_mode_fields(data);
                    done = true;
                }
            }
        });

        await sleep(1000);
    }

    if (callback) callback();
}