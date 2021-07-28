function set_mode_fields(data) {
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

    window.values_set = true;

    if (window.limits_set) validate_all();
}

function check_mode_write_success(newData) {
    if ((newData['speed'] != format_from_display($('#mode_speed').val())) ||
        (newData['turn1'] != format_from_display($('#mode_turn1').val())) ||
        (newData['turn2'] != format_from_display($('#mode_turn2').val())) ||
        (newData['brakeTime'] != format_from_display($('#mode_brake_time').val())) ||
        (newData['acc'] != format_from_display($('#mode_acc').val())) ||
        (newData['dec'] != format_from_display($('#mode_dec').val()))) {
        alert("Error writing values! Please try again!");
    }
}


async function set_mode_values() {
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

    $.ajax("modeValues", {
        data: JSON.stringify(val),
        contentType: 'application/json',
        type: 'POST',
        always: function (done) {
            setTimeout(get_mode_values(true), 600);
        }
    });
}

function validate_all() {

    // Only proceed if both limits and values are set
    if (window.limits && window.values) {
        // force update of all fields
        validate_speed();
        validate_turn1();
        validate_turn2();
        validate_brake_time();
        validate_acceleration();
        validate_deceleration();

        if (!validate_speed() ||
            !validate_turn1() ||
            !validate_turn2() ||
            !validate_brake_time() ||
            !validate_acceleration() ||
            !validate_deceleration()) {
            e = document.getElementById("set_button");
            setSaveButtonDisabled(e);
        }
        else {
            e = document.getElementById("set_button");
            setSaveButtonEnabled(e);
        }
    }
}

function validate_speed() {
    var e = document.getElementById("mode_speed");
    val = e.value
    if (isNaN(val) ||
        (format_from_display(val) > window.limits.maxSpeed) ||
        (format_from_display(val) < 0)) {
        setElementValueInvalid(e);
        return false;
    }
    else {
        setElementValueValid(e);
    }

    return true;
}

function validate_turn1() {
    var e = document.getElementById("mode_turn1");
    val = e.value
    if (isNaN(val) ||
        (format_from_display(val) >= window.limits.maxTurn1) ||
        (format_from_display(val) < window.limits.minTurn2) ||
        (format_from_display(val) < format_from_display($('#mode_turn2').val()))) {
        setElementValueInvalid(e);
        return false;
    }
    else {
        setElementValueValid(e);
    }

    return true;
}

function validate_turn2() {
    var e = document.getElementById("mode_turn2");
    val = e.value
    if (isNaN(val) ||
        (format_from_display(val) > window.limits.maxTurn1) ||
        (format_from_display(val) < window.limits.minTurn2) ||
        (format_from_display(val) > format_from_display($('#mode_turn1').val()))) {
        setElementValueInvalid(e);
        return false;
    } else {
        setElementValueValid(e);
    }

    return true;
}

function validate_brake_time() {
    // No limits on brake time

    return true;
}

function validate_acceleration() {
    var e = document.getElementById("mode_acc");
    val = e.value
    if (isNaN(val) ||
        (format_from_display(val) <= 0) ||
        (format_from_display(val) >= window.limits.maxAcc)) {
        setElementValueInvalid(e);
        return false;
    } else {
        setElementValueValid(e);
    }

    return true;
}

function validate_deceleration() {
    var e = document.getElementById("mode_dec");
    val = e.value
    if (isNaN(val) ||
        (format_from_display(val) <= 0) ||
        (format_from_display(val) >= window.limits.maxDec)) {
        setElementValueInvalid(e);
        return false;
    } else {
        setElementValueValid(e);
    }

    return true;
}

$(document).ready()
{
    setSaveButtonDisabled();
    get_mode_values(false, validate_all);
    get_limits(false, validate_all);
}
