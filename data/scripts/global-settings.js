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

function validate_all()
{
}

function set_limit_fields(data) {
    $('#limits_home').val(format_for_display(data["home"]));
    $('#limits_home_current').val(format_for_display(data["home"]));
    
    $('#limits_max_end').val(format_for_display(data["maxEnd"]));
    $('#limits_max_end_current').val(format_for_display(data["maxEnd"]));
    
    $('#limits_max_turn1').val(format_for_display(data["maxTurn1"]));
    $('#limits_max_turn1_current').val(format_for_display(data["maxTurn1"]));
    
    $('#limits_min_turn2').val(format_for_display(data["minTurn2"]));
    $('#limits_min_turn2_current').val(format_for_display(data["minTurn2"]));
    
    $('#limits_max_acc').val(format_for_display(data["maxAcc"]));
    $('#limits_max_acc_current').val(format_for_display(data["maxAcc"]));
    
    $('#limits_max_dec').val(format_for_display(data["maxDec"]));
    $('#limits_max_dec_current').val(format_for_display(data["maxDec"]));
    
    $('#limits_max_speed').val(format_for_display(data["maxSpeed"]));
    $('#limits_max_speed_current').val(format_for_display(data["maxSpeed"]));
    
    $('#limits_homing_speed').val(format_for_display(data["homingSpeed"]));
    $('#limits_homing_speed_current').val(format_for_display(data["homingSpeed"]));
    
    $('#limits_max_time').val(data["maxTime"]);
    $('#limits_max_time_current').val(data["maxTime"]);
    
    $('#limits_max_laps').val(data["maxLaps"]);
    $('#limits_max_laps_current').val(data["maxLaps"]);
    
    $('#limits_serv_speed').val(format_for_display(data["servSpeed"]));
    $('#limits_serv_speed_current').val(format_for_display(data["servSpeed"]));
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

    validate_all();
}

function save_limits()
{
    val = {
        'home' : format_from_display($('#global_home').val()),
        'maxEnd' : format_from_display($('#global_max_end').val()),
        'maxTurn1' : format_from_display($('#global_max_turn1').val()),
        'minTurn2' : format_from_display($('#global_min_turn2').val()),
        'maxAcc' : format_from_display($('#global_maxAcc').val()),
        'maxDec' : format_from_display($('#global_maxDec').val()),
        'maxSpeed' : format_from_display($('#global_maxSpeed').val()),
        'homingSpeed' : format_from_display($('#global_homingSpeed').val()),
        'maxTime' : $('#global_maxTime').val(),
        'maxLaps' : $('#global_maxLaps').val(),
        'servSpeed' : format_from_display($('#global_servSpeed').val())
    };

    $.ajax("globalValues", {
        data : JSON.stringify(val),
        contentType : 'application/json',
        type : 'POST'
        });
}

$(document).ready()
{
    get_limits();
}
