function updateStatus(data)
{
    console.log("Updating status values");

    $('#status_mode').text(data["mode"]);
    $('#status_completed_laps').text(data["completedLaps"]);
    $('#status_position').text(format_for_display(data["position"]));
    $('#status_system_status').text(data["systemStatus"]);
}

document.addEventListener('DOMContentLoaded', function() {
    console.log("status.js: Registering status handler");
    if (!window.socketEventHandlers) window.socketEventHandlers = {};
    window.socketEventHandlers["Status"] = updateStatus;

}, false);
