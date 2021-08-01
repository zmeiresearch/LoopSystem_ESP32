function updateStatus(data)
{
    $('#status_mode').text(data["mode"]);
    $('#status_completed_laps').text(data["completedLaps"]);
    $('#status_position').text(format_for_display(data["position"]));
    $('#status_system_status').text(data["systemStatus"]);
}

document.addEventListener('DOMContentLoaded', function() {
    console.log("status.js: Registering status handler");
    if (!window.socketMessageHandlers) window.socketMessageHandlers = {};
    window.socketMessageHandlers["Status"] = updateStatus;

}, false);
