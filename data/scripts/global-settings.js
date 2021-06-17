function update_values()
{
    //console.log("Updating values");

    $.get( "/globalValues", function( data ) {
        //console.log("Received: " + data);
        $('#global_home').val(data["home"]);
        $('#global_max_end').val(data["maxEnd"]);
        $('#global_max_turn1').val(data["maxTurn1"]);
        $('#global_min_turn2').val(data["minTurn2"]);
        $('#global_maxAcc').val(data["maxAcc"]);
        $('#global_maxDec').val(data["maxDec"]);
        $('#global_maxSpeed').val(data["maxSpeed"]);
        $('#global_homingSpeed').val(data["homingSpeed"]);
        $('#global_maxTime').val(data["maxTime"]);
        $('#global_maxLaps').val(data["maxLaps"]);
        $('#global_servSpeed').val(data["servSpeed"]); 
    });
}

function save_global_settings()
{
    val = {
        'home' : $('#global_home').val(),
        'maxEnd' : $('#global_max_end').val(),
        'maxTurn1' : $('#global_max_turn1').val(),
        'minTurn2' : $('#global_min_turn2').val(),
        'maxAcc' : $('#global_maxAcc').val(),
        'maxDec' : $('#global_maxDec').val(),
        'maxSpeed' : $('#global_maxSpeed').val(),
        'homingSpeed' : $('#global_homingSpeed').val(),
        'maxTime' : $('#global_maxTime').val(),
        'maxLaps' : $('#global_maxLaps').val(),
        'servSpeed' : $('#global_servSpeed').val()
    };

    $.ajax("globalValues", {
        data : JSON.stringify({"values": val}),
        contentType : 'application/json',
        type : 'POST'
        });
}

$(document).ready()
{
    update_values();
}
