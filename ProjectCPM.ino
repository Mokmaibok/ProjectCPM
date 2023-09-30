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

float initialAngle = 0.0;
int counter = 0;

void setup() {
  Serial.begin(115200);
  pinMode(MotorForward, OUTPUT);
  pinMode(MotorReverse, OUTPUT);
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
  controlMotors();
  
  if (isAutoButtonPressed) {
    isLimitSwitchForwardPressed = !digitalRead(LimitSwitchForward);
    isLimitSwitchReversePressed = !digitalRead(LimitSwitchReverse);
    if (isLimitSwitchForwardPressed) {
      stopMotor(); 
      delay(100);
      isStartButtonPressed = true;
      if (counter > 3) {
        isAutoButtonPressed = false;
        stopMotor();
        delay(500); 
        moveForward();
        delay(500); 
        stopMotor();
        delay(500); 
      }
    }
    
    if (isLimitSwitchReversePressed) {
      stopMotor();
      delay(100);
      isResetButtonPressed = true;
    }
  } else {
    counter = 0;
    checkLimitSwitches();
  }
}

void controlMotors() {
  if (isStartButtonPressed) moveForward();
  if (isStopButtonPressed) stopMotor();
  if (isResetButtonPressed) moveReverse();

  if (isButtonfixPosition) {
    initialAngle = getAngle();
    isButtonfixPosition = false;
  }

  float angle = getAngle() - initialAngle;

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
    delay(500);
  }
}

void moveForward() {
  digitalWrite(MotorForward, HIGH);
  digitalWrite(MotorReverse, LOW);
}

void moveReverse() {
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
        isAutoButtonPressed = false;
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
        isStartButtonPressed = true;
      }
    }
  }
}

void updatePublicAnto() {
  float angle = getAngle() - initialAngle;
  anto.pub("Angle", angle);
  anto.pub("Counter", counter);
}

void setupRoutes() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/index.html", HTTP_GET, handleRoot);
  server.on("/assets/css/style.css", HTTP_GET, []() { handleFile("/assets/css/style.css"); });
  server.on("/assets/js/main.js", HTTP_GET, []() { handleFile("/assets/js/main.js"); });
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
