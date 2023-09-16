const apiKey = "OnC9FwGsaEGSQVfoF28IH12607bJ5slL5hlIPCgX"
const thing = "Project_CPM"
const apiUrlGet = "https://api.anto.io/channel/get/" + apiKey + "/" + thing + "/"
const apiUrlSet = "https://api.anto.io/channel/set/" + apiKey + "/" + thing + "/"
let isButtonStartPressed = false
let isButtonStopPressed = false
let isButtonResetPressed = false
let isButton30Pressed = false
let isButton45Pressed = false
let isButton60Pressed = false

function updateDegree(name) {
    const url = apiUrlGet + name
    fetch(url)
        .then(response => response.json())
        .then(data => {
            const element = document.getElementById(name);
            if (element) {
                element.textContent = data.value;
            }
        })
        .catch(error => console.error('Error:', error))
}

setInterval(() => {
    updateDegree("Angle1");
}, 1000);

function sendCommand(name, nubmer) {
    const url = apiUrlSet + name + "/" + nubmer
    fetch(url)
        .then(response => response.json())
        .then(message => console.log(message))
        .catch(error => console.error('Error:', error))
}

function ButtonStart () {
    isButtonStartPressed = true
    sendCommand("ButtonStart", "1")
}

function ButtonStop () {
    isButtonStopPressed = true
    sendCommand("ButtonStop", "1")
}

function ButtonReset () {
    isButtonResetPressed = true
    sendCommand("ButtonReset", "1")
}

function ButtonStartReleased () {
    if (isButtonStartPressed) {
        sendCommand("ButtonStart", "0")
        isButtonStartPressed = false
    }
}

function ButtonStopReleased () {
    if (isButtonStopPressed) {
        sendCommand("ButtonStop", "0")
        isButtonStopPressed = false
    }
}

function ButtonResetReleased () {
    if (isButtonResetPressed) {
        sendCommand("ButtonReset", "0")
        isButtonResetPressed = false
    }
}

function Button30 () {
    isButton30Pressed = true
    sendCommand("Button30", "1")
}

function Button45 () {
    isButton45Pressed = true
    sendCommand("Button45", "1")
}

function Button60 () {
    isButton60Pressed = true
    sendCommand("Button60", "1")
}

function Button30Released () {
    if (isButton30Pressed) {
        sendCommand("Button30", "0")
        isButton30Pressed = false
    }
}

function Button45Released () {
    if (isButton45Pressed) {
        sendCommand("Button45", "0")
        isButton45Pressed = false
    }
}

function Button60Released () {
    if (isButton60Pressed) {
        sendCommand("Button60", "0")
        isButton60Pressed = false
    }
}