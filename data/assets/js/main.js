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
let isButtonFixPositionPressed = false
let isButtonUnFixPositionPressed = false
let isAutoButtonPressed = false
let isAutoButton30Pressed = false
let isAutoButton45Pressed = false
let isAutoButton60Pressed = false

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
    updateDegree("Angle");
    updateDegree("Counter");
    updateDegree("Target");
    updateDegree("PercentSpeed");
    updateDegree("StatusMode");
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

function ButtonFixPosition () {
    isButtonFixPositionPressed = true
    sendCommand("FixPosition", "1")
}

function ButtonFixPositionReleased () {
    if (isButtonFixPositionPressed) {
        sendCommand("FixPosition", "0")
        isButtonFixPositionPressed = false
    }
}

function ButtonManual () {
    isAutoButtonPressed = true
    sendCommand("ButtonAuto", "0")
}

function ButtonAuto () {
    isAutoButtonPressed = true
    sendCommand("ButtonAuto", "1")
}



document.getElementById('TargetForm').addEventListener('submit', function (e) {
    e.preventDefault(); // ป้องกันการรีโหลดหน้า

    // รับค่าจาก input
    var targetValue = document.getElementById('targetInput').value;

    // ส่งค่าไปยัง ESP8266 ผ่าน fetch
    fetch('/updateTarget', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/x-www-form-urlencoded'
        },
        body: 'target=' + targetValue
    })
    .then(function (response) {
        if (response.status === 200) {
            // การส่งสำเร็จ
            console.log('Target updated successfully.');
        } else {
            console.error('Failed to update target.');
        }
    })
    .catch(function (error) {
        console.error('Error:', error);
    });
});

document.getElementById('PWMForm').addEventListener('submit', function (e) {
    e.preventDefault();

    var pwmValue = document.getElementById('pwmSlider').value;
    var pwmPercentage = (pwmValue / 255) * 100;

    // แสดงค่า PWM เป็นเปอร์เซ็นต์
    document.getElementById('pwmValue').textContent = pwmPercentage.toFixed(2);

    // บันทึกค่า PWM ลงใน localStorage
    localStorage.setItem('pwmValue', pwmValue);

    // ส่งค่า PWM ไปยัง ESP8266 ผ่าน fetch
    fetch('/updatePWM', {
        method: 'POST',
        headers: {
            'Content-Type': 'application/x-www-form-urlencoded'
        },
        body: 'pwm=' + pwmValue
    })
    .then(function (response) {
        if (response.status === 200) {
            console.log('PWM value updated successfully.');
        } else {
            console.error('Failed to update PWM value.');
        }
    })
    .catch(function (error) {
        console.error('Error:', error);
    });
});

// เมื่อหน้าเว็บโหลดเสร็จแล้ว
document.addEventListener('DOMContentLoaded', function () {
    // ตรวจสอบว่ามีค่า PWM ที่ถูกบันทึกใน localStorage หรือไม่
    var storedPwmValue = localStorage.getItem('pwmValue');
    if (storedPwmValue !== null) {
        // แสดงค่า PWM เป็นเปอร์เซ็นต์
        var pwmPercentage = (storedPwmValue / 255) * 100;
        document.getElementById('pwmValue').textContent = pwmPercentage.toFixed(2);

        // อัพเดตค่า Slider ให้ตรงกับค่าที่ถูกบันทึก
        document.getElementById('pwmSlider').value = storedPwmValue;
    }
});
