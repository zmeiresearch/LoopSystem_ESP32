function updateValues()
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

function setPeriodicUpdate() {
    setInterval(updateValues, 1000);
}

$(document).ready(setPeriodicUpdate);
