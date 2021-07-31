function set_limit_fields(data = window.limits) {
    $('#limit_home').val(format_for_display(data["home"]));
    $('#limit_home_current').text(format_for_display(data["home"]));
    
    $('#limit_max_end').val(format_for_display(data["maxEnd"]));
    $('#limit_max_end_current').text(format_for_display(data["maxEnd"]));
    
    $('#limit_max_turn1').val(format_for_display(data["maxTurn1"]));
    $('#limit_max_turn1_current').text(format_for_display(data["maxTurn1"]));
    
    $('#limit_min_turn2').val(format_for_display(data["minTurn2"]));
    $('#limit_min_turn2_current').text(format_for_display(data["minTurn2"]));
    
    $('#limit_max_acc').val(format_for_display(data["maxAcc"]));
    $('#limit_max_acc_current').text(format_for_display(data["maxAcc"]));
    
    $('#limit_max_dec').val(format_for_display(data["maxDec"]));
    $('#limit_max_dec_current').text(format_for_display(data["maxDec"]));
    
    $('#limit_max_speed').val(format_for_display(data["maxSpeed"]));
    $('#limit_max_speed_current').text(format_for_display(data["maxSpeed"]));
    
    $('#limit_homing_speed').val(format_for_display(data["homingSpeed"]));
    $('#limit_homing_speed_current').text(format_for_display(data["homingSpeed"]));
    
    $('#limit_max_time').val(data["maxTime"]);
    $('#limit_max_time_current').text(data["maxTime"]);
    
    $('#limit_max_laps').val(data["maxLaps"]);
    $('#limit_max_laps_current').text(data["maxLaps"]);
    
    $('#limit_serv_speed').val(format_for_display(data["servSpeed"]));
    $('#limit_serv_speed_current').text(format_for_display(data["servSpeed"]));
}

function set_limits()
{
    val = {
        'home' : format_from_display($('#limit_home').val()),
        'maxEnd' : format_from_display($('#limit_max_end').val()),
        'maxTurn1' : format_from_display($('#limit_max_turn1').val()),
        'minTurn2' : format_from_display($('#limit_min_turn2').val()),
        'maxAcc' : format_from_display($('#limit_max_acc').val()),
        'maxDec' : format_from_display($('#limit_max_dec').val()),
        'maxSpeed' : format_from_display($('#limit_max_speed').val()),
        'homingSpeed' : format_from_display($('#limit_homing_speed').val()),
        'maxTime' : $('#limit_max_time').val(),
        'maxLaps' : $('#limit_max_laps').val(),
        'servSpeed' : format_from_display($('#limit_serv_speed').val())
    };

    $.ajax("globalValues", {
        data : JSON.stringify(val),
        contentType : 'application/json',
        type : 'POST'
        });
}

$(document).ready()
{
    get_limits(true, set_limit_fields);
}
