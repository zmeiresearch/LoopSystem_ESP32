
var values;
var valuesSet = false;
var limits;
var limitsSet = false;
var isSending = false;

/*
- Limits received:
	Is values received:
		- Yes
			- Validate
*/
function receiveLimits(data) {
    limits = data;
    limitsSet = true;
    if (valuesSet) {
        validateAll();
    }
}

/*
- Values received:
	Is first reception:
		- Yes: 
			- Update Current Values
			- Update Input Values
			Is Limits received:
				- Yes:
					- Validate
		- No:
			- Update Current Values
			If Values Stored from POST
				- Yes:
					- Are values same:
						- Notify success:
						- Notify failure:
*/
function receiveModeValues(data) {
    if (!valuesSet) {
        values = data;
        valuesSet = true;
        setFields(data);
    } else {
        if (isSending) {
            checkWriteSuccess(data);
        }
        isSending = false;
    }

    if (limitsSet) {
        validateAll();
    }
}

function setFields(data) {
    $('#mode_speed').val(format_for_display(data["speed"]));
    $('#mode_speed_current').text(format_for_display(data["speed"]));

    $('#mode_turn1').val(format_for_display(data["turn1"]));
    $('#mode_turn1_current').text(format_for_display(data["turn1"]));

    $('#mode_turn2').val(format_for_display(data["turn2"]));
    $('#mode_turn2_current').text(format_for_display(data["turn2"]));

    $('#mode_brake_time').val(format_for_display(data["brakeTime"]));
    $('#mode_brake_time_current').text(format_for_display(data["brakeTime"]));

    $('#mode_acc').val(format_for_display(data["acc"]));
    $('#mode_acc_current').text(format_for_display(data["acc"]));

    $('#mode_dec').val(format_for_display(data["dec"]));
    $('#mode_dec_current').text(format_for_display(data["dec"]));
}

function checkWriteSuccess(newData) {
    if ((newData['speed'] != format_from_display($('#mode_speed').val())) ||
        (newData['turn1'] != format_from_display($('#mode_turn1').val())) ||
        (newData['turn2'] != format_from_display($('#mode_turn2').val())) ||
        (newData['brakeTime'] != format_from_display($('#mode_brake_time').val())) ||
        (newData['acc'] != format_from_display($('#mode_acc').val())) ||
        (newData['dec'] != format_from_display($('#mode_dec').val()))) {
        alert("Error writing values! Please try again!");
    }
}


function uploadModeValues() {
    setSaveButtonDisabled(document.getElementById("set_button"));
    isSending = true;
    var mode = $('#mode_id').text();
    val = {
        "mode": mode,
        'speed': format_from_display($('#mode_speed').val()),
        'turn1': format_from_display($('#mode_turn1').val()),
        'turn2': format_from_display($('#mode_turn2').val()),
        'brakeTime': format_from_display($('#mode_brake_time').val()),
        'acc': format_from_display($('#mode_acc').val()),
        'dec': format_from_display($('#mode_dec').val()),
    };

    sendModeValues(val);

    setTimeout(function(){
        requestModeValues($('#mode_id').text());
    }, 500);
}

/*
- Input Values changed:
	- Is limits received:
		- Validate
*/
function validateAll() {
    e = document.getElementById("set_button");

    // Only proceed if both limits and values are set
    if (valuesSet && limitsSet) {
        // force update of all fields
        validateSpeed();
        validateTurn1();
        validateTurn2();
        validateBrakeTime();
        validateAcceleration();
        validateDeceleration();

        if (!validateSpeed() ||
            !validateTurn1() ||
            !validateTurn2() ||
            !validateBrakeTime() ||
            !validateAcceleration() ||
            !validateDeceleration()) {
            setSaveButtonDisabled(e);
        }
        else {
            setSaveButtonEnabled(e);
        }
    }
}

function validateSpeed() {
    var e = document.getElementById("mode_speed");
    val = e.value
    if (isNaN(val) ||
        (format_from_display(val) > limits.maxSpeed) ||
        (format_from_display(val) <= 0)) {
        setElementValueInvalid(e);
        return false;
    }
    else {
        setElementValueValid(e);
    }

    return true;
}

function validateTurn1() {
    var e = document.getElementById("mode_turn1");
    val = e.value
    if (isNaN(val) ||
        (format_from_display(val) >= limits.maxTurn1) ||
        (format_from_display(val) < limits.minTurn2) ||
        (format_from_display(val) < format_from_display($('#mode_turn2').val()))) {
        setElementValueInvalid(e);
        return false;
    }
    else {
        setElementValueValid(e);
    }

    return true;
}

function validateTurn2() {
    var e = document.getElementById("mode_turn2");
    val = e.value
    if (isNaN(val) ||
        (format_from_display(val) > limits.maxTurn1) ||
        (format_from_display(val) < limits.minTurn2) ||
        (format_from_display(val) > format_from_display($('#mode_turn1').val()))) {
        setElementValueInvalid(e);
        return false;
    } else {
        setElementValueValid(e);
    }

    return true;
}

function validateBrakeTime() {
    // No limits on brake time

    return true;
}

function validateAcceleration() {
    var e = document.getElementById("mode_acc");
    val = e.value
    if (isNaN(val) ||
        (format_from_display(val) <= 0) ||
        (format_from_display(val) > limits.maxAcc)) {
        setElementValueInvalid(e);
        return false;
    } else {
        setElementValueValid(e);
    }

    return true;
}

function validateDeceleration() {
    var e = document.getElementById("mode_dec");
    val = e.value
    if (isNaN(val) ||
        (format_from_display(val) <= 0) ||
        (format_from_display(val) > limits.maxDec)) {
        setElementValueInvalid(e);
        return false;
    } else {
        setElementValueValid(e);
    }

    return true;
}

/*
- Page Loaded:
	- Disable Button
	- Disable Fields
	- Request Values
	- Request Limits
*/
document.addEventListener('DOMContentLoaded', function() {
    console.log("mode.js: Registering handlers");
    if (!window.socketMessageHandlers) window.socketMessageHandlers = {};
    window.socketMessageHandlers["ModeValues"] = receiveModeValues;
    window.socketMessageHandlers["GlobalValues"] = receiveLimits;

    //if (!window.socketCloseHandlers) window.socketCloseHandlers = {};
    //window.socketCloseHandlers["mode"] = socketClosed;

    setSaveButtonDisabled(document.getElementById("set_button"));

    requestModeValues($('#mode_id').text());
    requestGlobalValues();

}, false);

