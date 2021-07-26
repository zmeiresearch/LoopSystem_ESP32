function sleep(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
  }

function format_for_display(val)
{
    return (val/10).toFixed(1);
}

function format_from_display(val)
{
    return parseInt((val*10).toFixed(0));
}

function set_mode_fields(data){
    $('#mode_speed').val(format_for_display(data["speed"]));
    $('#mode_speed_current').text(format_for_display(data["speed"]));

    $('#mode_turn1').val(format_for_display(data["turn1"]));
    $('#mode_turn1_current').text(format_for_display(data["turn1"]));
    
    $('#mode_turn2').val(format_for_display(data["turn2"]));
    $('#mode_turn2_current').text(format_for_display(data["turn2"]));
    
    $('#mode_brake_time').val(format_for_display(data["brakeTime"]));
    $('#mode_brake_time_current').text(format_for_display(data["brakeTime"]));
    
    $('#mode_acc').val(format_for_display(data["acc"]));
    $('#mode_acc_current').text(format_for_display(data["acc"]));
    
    $('#mode_dec').val(format_for_display(data["dec"]));
    $('#mode_dec_current').text(format_for_display(data["dec"]));

    window.values_set = true;

    if (window.limits_set) validate_all();
}

function check_write_success(newData) {
    if ((newData['speed']       != format_from_display($('#mode_speed').val()))     ||
        (newData['turn1']       != format_from_display($('#mode_turn1').val()))     ||
        (newData['turn2']       != format_from_display($('#mode_turn2').val()))     ||
        (newData['brakeTime']   != format_from_display($('#mode_brake_time').val()))||
        (newData['acc']         != format_from_display($('#mode_acc').val()))       ||
        (newData['dec']         != format_from_display($('#mode_dec').val())) )
        {
            alert("Error writing values! Please try again!");
        }
}

async function get_mode_values(checkWrite)
{
    var done = false;
    while (!done)
    {
        var mode = $('#mode_id').text();
        $.ajax({
            type: "GET",
            data: {'mode' : mode },
            url: "/modeValues",
            success: function(data) {
                if (checkWrite) {
                    check_write_success(data);
                } else {
                    set_mode_fields(data);
                }
                                
                done = true;
            },
            /*error: function(response) {
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
            }*/
        });

        await sleep(1000);
    }
}

async function set_mode_values()
{
    var mode = $('#mode_id').text();
    val = {
        "mode" : mode,
        'speed' : format_from_display($('#mode_speed').val()),
        'turn1' : format_from_display($('#mode_turn1').val()),
        'turn2' : format_from_display($('#mode_turn2').val()),
        'brakeTime' : format_from_display($('#mode_brake_time').val()),
        'acc' : format_from_display($('#mode_acc').val()),
        'dec' : format_from_display($('#mode_dec').val()),
    };

    $.ajax("modeValues", {
        data : JSON.stringify(val),
        contentType : 'application/json',
        type : 'POST',
        always : function(done) {
            setTimeout(get_mode_values(true), 600);
        }});
}

async function get_limits()
{
    var done = false;
    while (!done)
    {
        $.ajax({
            type: "GET",
            url: "/globalValues",
            success: function(data) {
                window.limits = data;
                done = true;
            },
            /*error: function(response) {
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
            }*/
        });

        await sleep(1000);
    }

    window.limits_set = true;

    if (window.values_set) validate_all();
}

function setElementValueValid(e) {
    e.classList.remove("value_invalid");
    e.classList.add("value_valid");
}

function setElementValueInvalid(e) {
    e.classList.remove("value_valid");
    e.classList.add("value_invalid");
}

function validate_all(){
    e = document.getElementById("set_button");

    // force update of all fields
    validate_speed();
    validate_turn1();
    validate_turn2();
    validate_brake_time();
    validate_acceleration();
    validate_deceleration();

    if (!validate_speed() ||
        !validate_turn1() ||
        !validate_turn2() ||
        !validate_brake_time() ||
        !validate_acceleration() ||
        !validate_deceleration())
    {
        e.classList.remove("save_button_enabled");
        e.classList.add("save_button_disabled");
    }
    else
    {
        e.classList.remove("save_button_disabled");
        e.classList.add("save_button_enabled");
    }
}

function validate_speed() {
    var e = document.getElementById("mode_speed");
    val = e.value
    if ( isNaN(val) || 
        (format_from_display(val) > window.limits.maxSpeed) ||
        (format_from_display(val) < 0))
    {
        setElementValueInvalid(e);
        return false;
    }
    else
    {
        setElementValueValid(e);
    }

    return true;
}

function validate_turn1() {
    var e = document.getElementById("mode_turn1");
    val = e.value
    if ( isNaN(val) || 
        (format_from_display(val) >= window.limits.maxTurn1) ||
        (format_from_display(val) < window.limits.minTurn2) ||
        (format_from_display(val) < format_from_display($('#mode_turn2').val())))
    {
        setElementValueInvalid(e);
        return false;
    }
    else
    {
        setElementValueValid(e);
    }

    return true;
}

function validate_turn2() {
    var e = document.getElementById("mode_turn2");
    val = e.value
    if ( isNaN(val) || 
        (format_from_display(val) > window.limits.maxTurn1) ||
        (format_from_display(val) < window.limits.minTurn2) ||
        (format_from_display(val) > format_from_display($('#mode_turn1').val()))) 
    {
        setElementValueInvalid(e);
        return false;
    } else {
        setElementValueValid(e);
    }

    return true;
}

function validate_brake_time() {
    /*val = this.value
    if ( isNaN(val) || 
        (format_from_display(val) > window.limits.maxTurn1) ||
        (format_from_display(val) < window.limits.minTurn2) ||
        (format_from_display(val) < format_from_display($('#mode_turn2').val())))
    {
        setElementValueInvalid(this);
        return false;
    }
    else
    {
        setElementValueValid(this);
    }*/

    return true;
}

function validate_acceleration() {
    var e = document.getElementById("mode_acc");
    val = e.value
    if ( isNaN(val) || 
        (format_from_display(val) <= 0) ||
        (format_from_display(val) >= window.limits.maxAcc))
    {
        setElementValueInvalid(e);
        return false;
    } else {
        setElementValueValid(e);
    }

    return true;
}

function validate_deceleration() {
    var e = document.getElementById("mode_dec");
    val = e.value
    if ( isNaN(val) || 
        (format_from_display(val) <= 0) ||
        (format_from_display(val) >= window.limits.maxDec))
    {
        setElementValueInvalid(e);
        return false;
    } else {
        setElementValueValid(e);
    }

    return true;
}

$(document).ready()
{
    get_mode_values(false);
    get_limits();

    //$('#mode_speed').change(handle_speed_change);

}
