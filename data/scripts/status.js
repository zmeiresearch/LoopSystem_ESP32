function format_for_display(val)
{
    return (val/10).toFixed(1);
}

function updateStatus()
{
    //console.log("Updating values");

    $.get( "/status", function( data ) {
        //console.log("Received: " + data);
        $('#status_mode').text(data["mode"]);
        $('#status_completed_laps').text(data["completedLaps"]);
        $('#status_position').text(format_for_display(data["position"]));
        $('#status_system_status').text(data["systemStatus"]);
    });

}

function setPeriodicUpdate() {
    setInterval(updateStatus, 1000);
}

$(document).ready(setPeriodicUpdate);
