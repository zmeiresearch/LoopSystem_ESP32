function format_for_display(val)
{
    return (val/10).toFixed(1);
}

function format_from_display(val)
{
    return parseInt((val*10).toFixed(0));
}

function update_values()
{
    //console.log("Updating values");

    $.get( "/globalValues", function( data ) {
        //console.log("Received: " + data);
        $('#global_home').val(format_for_display(data["home"]));
        $('#global_max_end').val(format_for_display(data["maxEnd"]));
        $('#global_max_turn1').val(format_for_display(data["maxTurn1"]));
        $('#global_min_turn2').val(format_for_display(data["minTurn2"]));
        $('#global_maxAcc').val(format_for_display(data["maxAcc"]));
        $('#global_maxDec').val(format_for_display(data["maxDec"]));
        $('#global_maxSpeed').val(format_for_display(data["maxSpeed"]));
        $('#global_homingSpeed').val(format_for_display(data["homingSpeed"]));
        $('#global_maxTime').val(data["maxTime"]);
        $('#global_maxLaps').val(data["maxLaps"]);
        $('#global_servSpeed').val(format_for_display(data["servSpeed"]));
    });
}

function save_global_settings()
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
    update_values();
}
