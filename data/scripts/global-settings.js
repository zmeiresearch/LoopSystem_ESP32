function update_values()
{
    //console.log("Updating values");

    $.get( "/globalValues", function( data ) {
        //console.log("Received: " + data);
        $('#global_home').val(data["home"]);
        $('#global_end').val(data["end"]);
        $('#global_turn1').val(data["turn1"]);
        $('#global_turn2').val(data["turn2"]);
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
        'end' : $('#global_end').val(),
        'turn1' : $('#global_turn1').val(),
        'turn2' : $('#global_turn2').val(),
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

function set_periodic_update() {
    setInterval(updateValues, 1000);
}

$(document).ready(setPeriodicUpdate)
{
    update_values();
}
