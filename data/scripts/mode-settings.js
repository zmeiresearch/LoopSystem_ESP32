function sleep(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
  }

async function update_mode_values()
{
    var done = false;

    while (!done)
    {
        //console.log("Updating values");
        
        var mode = $('#mode_id').text();

        $.get( "/modeValues", { 'mode' : mode }, function( data ) {
            //console.log("Received: " + data);
            $('#mode_speed').val(data["speed"]);
            $('#mode_turn1').val(data["turn1"]);
            $('#mode_turn2').val(data["turn2"]);
            $('#mode_brake_time').val(data["brakeTime"]);
            $('#mode_acc').val(data["acc"]);
            $('#mode_dec').val(data["dec"]);
            
            
            done = true;
        });
        await sleep(200);
    }
}

async function save_mode_values()
{
    var mode = $('#mode_id').text();
    val = {
        'speed' : $('#mode_speed').val(),
        'turn1' : $('#mode_turn1').val(),
        'turn2' : $('#mode_turn2').val(),
        'brakeTime' : $('#mode_brake_time').val(),
        'acc' : $('#mode_acc').val(),
        'dec' : $('#mode_dec').val(),
    };

    $.ajax("modeValues", {
        data : JSON.stringify({"mode" : mode, "values": val}),
        contentType : 'application/json',
        type : 'POST'
        }).done(function() {
            setTimeout(update_mode_values, 1000);
          });

}

$(document).ready()
{
    update_mode_values();
}
