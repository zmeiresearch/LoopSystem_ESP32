function sleep(ms) {
    return new Promise(resolve => setTimeout(resolve, ms));
  }

function format_for_display(val)
{
    return (val/10).toFixed(1);
}

function format_from_display(val)
{
    return (val*10).toFixed(0);
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
            $('#mode_speed').val(format_for_display(data["speed"]));
            $('#mode_turn1').val(format_for_display(data["turn1"]));
            $('#mode_turn2').val(format_for_display(data["turn2"]));
            $('#mode_brake_time').val(format_for_display(data["brakeTime"]));
            $('#mode_acc').val(format_for_display(data["acc"]));
            $('#mode_dec').val(format_for_display(data["dec"]));
            
            done = true;
        });
        await sleep(200);
    }
}

async function save_mode_values()
{
    var mode = $('#mode_id').text();
    val = {
        'speed' : format_from_display($('#mode_speed').val()),
        'turn1' : format_from_display($('#mode_turn1').val()),
        'turn2' : format_from_display($('#mode_turn2').val()),
        'brakeTime' : format_from_display($('#mode_brake_time').val()),
        'acc' : format_from_display($('#mode_acc').val()),
        'dec' : format_from_display($('#mode_dec').val()),
    };

    $.ajax("modeValues", {
        data : JSON.stringify({"mode" : mode, "values": val}),
        contentType : 'application/json',
        type : 'POST'
        }).always(function(done) {
            setTimeout(update_mode_values, 600);
        });
}

$(document).ready()
{
    update_mode_values();
}
