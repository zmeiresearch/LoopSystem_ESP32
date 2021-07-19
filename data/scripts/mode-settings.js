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

function update_limits()
{
    //console.log("Updating values");

    $.get( "/globalValues", function( data ) {
        window.limits = data;
        //console.log("Received: " + data);
        //$('#global_home'.val(data["home"]);
        //$('#global_max_end').val(data["maxEnd"]);
        //$('#global_max_turn1').val(data["maxTurn1"]);
        //$('#global_min_turn2').val(data["minTurn2"]);
        //$('#global_maxAcc').val(data["maxAcc"]);
        //$('#global_maxDec').val(data["maxDec"]);
        //$('#global_maxSpeed').val(data["maxSpeed"]);
        //$('#global_homingSpeed').val(data["homingSpeed"]);
        //$('#global_maxTime').val(data["maxTime"]);
        //$('#global_maxLaps').val(data["maxLaps"]);
        //$('#global_servSpeed').val(data["servSpeed"]); 
    });
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

function handle_speed_change(e)
{
    speed = $('#mode_speed')
    val = speed.val();
    if ( isNaN(val) || 
        (format_from_display(val) > window.limits.maxSpeed) ||
        (format_from_display(val) < window.limits.homingSpeed))
    {
        speed.css('background', '#FF0000');
    }
    else
    {
        speed.css('background', '#FFFFFF');
    }
}

$(document).ready()
{
    update_mode_values();
    update_limits();

    //$('#mode_speed').change(handle_speed_change);

}
