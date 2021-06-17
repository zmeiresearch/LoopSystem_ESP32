function update_mode_values(mode)
{
    //console.log("Updating values");

    $.get( "/modeValues", { 'mode' : mode }, function( data ) {
        //console.log("Received: " + data);
        $('#mode_end').val(data["end"]);
        $('#mode_acc').val(data["acc"]);
        $('#mode_dec').val(data["dec"]);
        $('#mode_turn').val(data["turn"]);
    });

}

function save_mode_values()
{
    val = {
        'end' : $('#mode_end').val(),
        'acc' : $('#mode_acc').val(),
        'dec' : $('#mode_dec').val(),
        'turn' : $('#mode_turn').val(),
    };

    $.ajax("modeValues", {
        data : JSON.stringify({"mode" : mode, "values": val}),
        contentType : 'application/json',
        type : 'POST'
        });
}

function set_periodic_update() {
    setInterval(updateValues, 1000);
}

$(document).ready(setPeriodicUpdate)
{
    var mode = $('#mode_id').text();
    update_mode_values(mode);
}
