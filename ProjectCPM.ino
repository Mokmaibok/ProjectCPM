#include <ESP8266WebServer.h>
#include <LittleFS.h>
#include <AntoIO.h>
#include <Wire.h>
#include <MPU6050.h>

const char *ssid = "หมก";
const char *password = "csos1478";
const char *user = "Rmutt_Mec_CPM";
const char *token = "OnC9FwGsaEGSQVfoF28IH12607bJ5slL5hlIPCgX";
const char *thing = "Project_CPM";

AntoIO anto(user, token, thing);
ESP8266WebServer server(80);
MPU6050 mpu;

#define MotorForward 13
#define MotorReverse 15
#define SpeedMotor 0
#define LimitSwitchForward 14
#define LimitSwitchReverse 12

bool isStartButtonPressed = false;
bool isStopButtonPressed = false;
bool isResetButtonPressed = false;
bool isButton30Pressed = false;
bool isButton45Pressed = false;
bool isButton60Pressed = false;
bool isLimitSwitchForwardPressed = false;
bool isLimitSwitchReversePressed = false;
bool isButtonfixPosition = false;
bool isAutoButtonPressed = false;
bool wasLimitSwitchForwardPressed = false;

float initialAngle = 0.0;
int counter = 0;
int target = 3;
int pwmspeedmotor = 255;
String statusmode = "Manual";

void setup() {
  Serial.begin(115200);
  pinMode(MotorForward, OUTPUT);
  pinMode(MotorReverse, OUTPUT);
  pinMode(SpeedMotor, OUTPUT);
  pinMode(LimitSwitchForward, INPUT_PULLUP);
  pinMode(LimitSwitchReverse, INPUT_PULLUP);
  setupAnto();
  LittleFS.begin();
  server.begin();
  Wire.begin();
  mpu.initialize();
  setupRoutes();
  initialAngle = getAngle();
}

void loop() {
  server.handleClient();
  anto.mqtt.loop();
  updatePublicAnto();
  
  if (isAutoButtonPressed) {
    statusmode = "Auto";
    float angle = getAngle() - initialAngle;
  
    if (isButtonfixPosition) {
      initialAngle = getAngle();
      isButtonfixPosition = false;
    }
    
    if (isStartButtonPressed) moveForward();
    if (isStopButtonPressed) stopMotor();
    if (isResetButtonPressed) moveReverse();

    if (isButton30Pressed || isButton45Pressed || isButton60Pressed) {
      isStartButtonPressed = true;
      
      if ((isButton30Pressed && angle >= 30) || (isButton45Pressed && angle >= 45) || (isButton60Pressed && angle >= 60)) {
        isStartButtonPressed = false;
        isResetButtonPressed = true;
      }
    }
    
    isLimitSwitchForwardPressed = !digitalRead(LimitSwitchForward);
    isLimitSwitchReversePressed = !digitalRead(LimitSwitchReverse);
    
    if (isLimitSwitchReversePressed) {
      isStartButtonPressed = false;
      isResetButtonPressed = true;
    }

    if (isLimitSwitchForwardPressed && !wasLimitSwitchForwardPressed) {
      isStartButtonPressed = true;
      isResetButtonPressed = false;
      counter++;
    }
    
    if (counter > target) {
      isStopButtonPressed = true;
      checkLimitSwitches();
      counter = 0;
      wasLimitSwitchForwardPressed = false;
    }
    
    wasLimitSwitchForwardPressed = isLimitSwitchForwardPressed;
    
  } else {
    manualMode();
    checkLimitSwitches();
    counter = 0;
    wasLimitSwitchForwardPressed = false;
  }
}

void manualMode() {
  statusmode = "Manual";
  float angle = getAngle() - initialAngle;

  if (isButtonfixPosition) {
    initialAngle = getAngle();
    isButtonfixPosition = false;
  }
  
  if (isStartButtonPressed) moveForward();
  if (isStopButtonPressed) stopMotor();
  if (isResetButtonPressed) moveReverse();

  if (isButton30Pressed || isButton45Pressed || isButton60Pressed) {
    moveForward();
    if ((isButton30Pressed && angle >= 30) || (isButton45Pressed && angle >= 45) || (isButton60Pressed && angle >= 60))
      stopMotor();
  }
}

void checkLimitSwitches() {
  isLimitSwitchForwardPressed = !digitalRead(LimitSwitchForward);
  isLimitSwitchReversePressed = !digitalRead(LimitSwitchReverse);

  if (isLimitSwitchForwardPressed || isLimitSwitchReversePressed) {
    stopMotor();
    delay(500);
    if (isLimitSwitchForwardPressed) moveForward();
    else moveReverse();
    delay(500);
    stopMotor();
  }
}

void moveForward() {
  analogWrite(SpeedMotor, pwmspeedmotor);
  digitalWrite(MotorForward, HIGH);
  digitalWrite(MotorReverse, LOW);
}

void moveReverse() {
  analogWrite(SpeedMotor, pwmspeedmotor);
  digitalWrite(MotorForward, LOW);
  digitalWrite(MotorReverse, HIGH);
}

void stopMotor() {
  digitalWrite(MotorForward, LOW);
  digitalWrite(MotorReverse, LOW);
  isStartButtonPressed = false;
  isStopButtonPressed = false;
  isResetButtonPressed = false;
  isButton30Pressed = false;
  isButton45Pressed = false;
  isButton60Pressed = false;
}

float getAngle() {
  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  return atan2(ay, az) * 180.0 / PI;
}

void setupAnto() {
  Serial.print(F("Anto library version: "));
  Serial.println(anto.getVersion());
  Serial.print(F("Trying to connect to "));
  Serial.println(ssid);
  anto.begin(ssid, password, messageReceived);
  Serial.println(F("Connected to Anto"));
  anto.sub("ButtonStart");
  anto.sub("ButtonStop");
  anto.sub("ButtonReset");
  anto.sub("Button30");
  anto.sub("Button45");
  anto.sub("Button60");
  anto.sub("FixPosition");
  anto.sub("ButtonAuto");
}

void messageReceived(String thing, String channel, String payload) {
  Serial.print(F("Received: "));
  Serial.print(thing);
  Serial.print(F("/"));
  Serial.print(channel);
  Serial.print(F("-> "));
  Serial.println(payload);
  if (thing == "Project_CPM") {
    if (channel == "ButtonStart") {
      if (payload == "1") {
        isStartButtonPressed = true;
      }
    } else if (channel == "ButtonStop") {
      if (payload == "1") {
        isStopButtonPressed = true;
      }
    } else if (channel == "ButtonReset") {
      if (payload == "1") {
        isResetButtonPressed = true;
      }
    } else if (channel == "Button30") {
      if (payload == "1") {
        isButton30Pressed = true;
      }
    } else if (channel == "Button45") {
      if (payload == "1") {
        isButton45Pressed = true;
      }
    } else if (channel == "Button60") {
      if (payload == "1") {
        isButton60Pressed = true;
      }
    } else if (channel == "FixPosition") {
      if (payload == "1") {
        isButtonfixPosition = true;
      }
    } else if (channel == "ButtonAuto") {
      if (payload == "1") {
        isAutoButtonPressed = true;
      } else if (payload == "0") {
        isAutoButtonPressed = false;
      }
    }
  }
}

void updatePublicAnto() {
  float angle = getAngle() - initialAngle;
  
  anto.pub("Angle", angle);
  anto.pub("Counter", counter);
  anto.pub("Target", target);
  anto.pub("StatusMode", statusmode);
}

// ฟังก์ชันสำหรับอัพเดต Target
void handleupdateTarget() {
  if (server.hasArg("target")) {
      target = server.arg("target").toInt();
      server.send(200, "text/plain", "Target updated");
  } else {
      server.send(400, "text/plain", "Bad Request");
  }
}

// ฟังก์ชันสำหรับอัพเดต PWM Speed Motor
void handleupdatePWM() {
  if (server.hasArg("pwm")) {
      pwmspeedmotor = server.arg("pwm").toInt();
      server.send(200, "text/plain", "PWM value updated");
  } else {
      server.send(400, "text/plain", "Bad Request");
  }
}

void setupRoutes() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/index.html", HTTP_GET, handleRoot);
  server.on("/assets/css/style.css", HTTP_GET, []() { handleFile("/assets/css/style.css"); });
  server.on("/assets/js/main.js", HTTP_GET, []() { handleFile("/assets/js/main.js"); });
  server.on("/updateTarget", HTTP_POST, handleupdateTarget);
  server.on("/updatePWM", HTTP_POST, handleupdatePWM);

}

void handleRoot() {
  handleFile("/index.html");
}

String getContentType(const String &fileName) {
  if (fileName.endsWith(".html")) return "text/html";
  if (fileName.endsWith(".css")) return "text/css";
  if (fileName.endsWith(".js")) return "application/javascript";
  if (fileName.endsWith(".jpg") || fileName.endsWith(".jpeg")) return "image/jpeg";
  if (fileName.endsWith(".png")) return "image/png";
  if (fileName.endsWith(".gif")) return "image/gif";
  if (fileName.endsWith(".ico")) return "image/x-icon";
  if (fileName.endsWith(".xml")) return "text/xml";
  if (fileName.endsWith(".pdf")) return "application/pdf";
  if (fileName.endsWith(".zip")) return "application/zip";
  return "text/plain";
}

void handleFile(const String& fileName) {
  File file = LittleFS.open(fileName, "r");
  if (!file) {
    server.send(404, "text/plain", "File not found");
    return;
  }
  server.streamFile(file, getContentType(fileName));
  file.close();
}
