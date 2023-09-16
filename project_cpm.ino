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

// สร้างอินสแตนซ์ของ AntoIO ด้วยข้อมูลผู้ใช้งานและเครื่อง Anto ที่เซ็ตไว้
AntoIO anto(user, token, thing);

// สร้างอินสแตนซ์ของ ESP8266WebServer
ESP8266WebServer server(80);

MPU6050 mpu1;
MPU6050 mpu2;

#define SDA_PIN1  5 // กำหนดขา GPIO SDA สำหรับ MPU6050 ตัวที่ 1
#define SCL_PIN1 4 // กำหนดขา GPIO SCL สำหรับ MPU6050 ตัวที่ 1

#define SDA_PIN2 0 // กำหนดขา GPIO SDA สำหรับ MPU6050 ตัวที่ 2
#define SCL_PIN2 2 // กำหนดขา GPIO SCL สำหรับ MPU6050 ตัวที่ 2

// กำหนดขาของมอเตอร์และสวิตช์ของคุณ
#define MotorForward 13
#define MotorReverse 15
#define LimitSwitchForward 14
#define LimitSwitchReverse 12

// สร้างตัวแปรเพื่อตรวจสอบสถานะของสวิตช์
bool isStartButtonPressed = false;
bool isStopButtonPressed = false;
bool isResetButtonPressed = false;
bool isButton30Pressed = false;
bool isButton45Pressed = false;
bool isButton60Pressed = false;
bool isLimitSwitchForwardPressed = false;
bool isLimitSwitchReversePressed = false;

void setup() {
  // เริ่มการสื่อสารผ่าน Serial
  Serial.begin(115200);
  
  // กำหนดขาของมอเตอร์และสวิตช์เป็น INPUT และกำหนดให้ใช้ Pull-Up Resistor
  pinMode(MotorForward, OUTPUT);
  pinMode(MotorReverse, OUTPUT);
  pinMode(LimitSwitchForward, INPUT_PULLUP);
  pinMode(LimitSwitchReverse, INPUT_PULLUP);
  
  // ตั้งค่าการเชื่อมต่อ Wi-Fi และ Anto
  setupAnto();
  
  // เริ่มต้น SPIFFS, เซิร์ฟเวอร์ และ Wire
  LittleFS.begin();
  server.begin();
  Wire.begin();
  
  mpu1.initialize();
  mpu2.initialize();
  
  // ตั้งค่า Route สำหรับ HTTP requests
  setupRoutes();
}

void loop() {
  
  // อัพเดตสถานะของเว็บเซิร์ฟเวอร์
  server.handleClient();
  
  //ใช้ฟังก์ชัน `loop()` ของ Anto MQTT ในการรับข้อมูลจาก Anto
  anto.mqtt.loop();
  
  //ใช้ฟังก์ชันอัพเดตค่าจาก MPU6050 และส่งข้อมูลไปยัง Anto
  updateDegreeAngle();
  
  if (isStartButtonPressed) {
    moveForward();
  }
  
  if (isStopButtonPressed) {
    stopMotor();
  }

  if (isResetButtonPressed) {
    moveReverse();
  }
  
  // อ่านค่ามุมองศาจาก MPU6050
  float angle1 = getAngle(&mpu1);
  float angle2 = getAngle(&mpu2);
  
  if (isButton30Pressed) {
    moveForward();
    if (angle1 >= 30){
      stopMotor();
    }
  }

  if (isButton45Pressed) {
    moveForward();
    if (angle1 >= 45){
      stopMotor();
    }
  }

  if (isButton60Pressed) {
    moveForward();
    if (angle1 >= 60){
      stopMotor();
    }
  }

  // ตรวจสอบสถานะของสวิตช์และทำการปรับปรุงสถานะมอเตอร์ตามเงื่อนไข
  isLimitSwitchForwardPressed = !digitalRead(LimitSwitchForward);
  isLimitSwitchReversePressed = !digitalRead(LimitSwitchReverse);

  //รับค่าจาก isLimitSwitchForwardPressed เพื่อสั่งให้มอเตอร์หยุดการทำงาน
  if (isLimitSwitchForwardPressed) {
    stopMotor(); // หยุดมอเตอร์
    delay(500);
    moveForward(); // ขับเคลื่อนมอเตอร์ไปข้างหน้า
    delay(500);
    stopMotor(); // หยุดมอเตอร์
    delay(500);
  }

  //รับค่าจาก isLimitSwitchReversePressed เพื่อสั่งให้มอเตอร์หยุดการทำงาน
  if (isLimitSwitchReversePressed) {
    stopMotor(); // หยุดมอเตอร์
    delay(500);
    moveReverse(); // ขับเคลื่อนมอเตอร์ถอยหลัง
    delay(500);
    stopMotor(); // หยุดมอเตอร์
    delay(500);
  }
}

// ฟังก์ชันสำหรับขับเคลื่อนมอเตอร์ไปข้างหน้า
void moveForward() {
  digitalWrite(MotorForward, HIGH);
  digitalWrite(MotorReverse, LOW);
}

// ฟังก์ชันสำหรับขับเคลื่อนมอเตอร์ถอยหลัง
void moveReverse() {
  digitalWrite(MotorForward, LOW);
  digitalWrite(MotorReverse, HIGH);
}

// ฟังก์ชันสำหรับหยุดมอเตอร์
void stopMotor() {
  isStartButtonPressed = false;
  isStopButtonPressed = false;
  isResetButtonPressed = false;
  isButton30Pressed = false;
  isButton45Pressed = false;
  isButton60Pressed = false;
  digitalWrite(MotorForward, LOW);
  digitalWrite(MotorReverse, LOW);
}

float getAngle(MPU6050 *mpu) {
  // อ่านค่าข้อมูลจาก MPU6050
  int16_t ax, ay, az, gx, gy, gz;
  mpu->getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  // คำนวณค่า roll
  float angle = atan2(ay, az) * 180.0 / PI;

  return angle;
}

// ฟังก์ชันในการตั้งค่าเป็น Station เชื่อมต่อเครือข่าย Wi-Fi
void setupAnto() {
  Serial.print("Anto library version: ");
  Serial.println(anto.getVersion());

  Serial.print("Trying to connect to ");
  Serial.println(ssid);

  anto.begin(ssid, password, messageReceived);

  Serial.println("Connected to Anto");

  anto.sub("ButtonStart");
  anto.sub("ButtonStop");
  anto.sub("ButtonReset");
  anto.sub("Button30");
  anto.sub("Button45");
  anto.sub("Button60");
}

// ฟังก์ชันในการรับข้อมูลที่ถูกส่งมาจาก Anto MQTT
void messageReceived(String thing, String channel, String payload) {
  Serial.print("Received: ");
  Serial.print(thing);
  Serial.print("/");
  Serial.print(channel);
  Serial.print("-> ");
  Serial.println(payload);
  
  // ตรวจสอบว่ารับข้อมูลจากช่องที่เราสนใจหรือไม่
  if (thing == "Project_CPM") {
    if (channel == "ButtonStart") {
      // ถ้ารับค่า "1" ให้มอเตอร์ขับเคลื่อนไปข้างหน้า
      if (payload == "1") {
        isStartButtonPressed = true;
      }
    } else if (channel == "ButtonStop") {
      // ถ้ารับค่า "1" ให้เปิดมอเตอร์หยุดการทำงาน
      if (payload == "1") {
        isStopButtonPressed = true;
      }
    } else if (channel == "ButtonReset") {
      // ถ้ารับค่า "1" ให้มอเตอร์ขับเคลื่อนไปด้านหลัง
      if (payload == "1") {
        isResetButtonPressed = true;
      }
    } else if (channel == "Button30") {
      // ถ้ารับค่า "1" ให้มอเตอร์ขับเคลื่อนไปด้านหลัง
      if (payload == "1") {
        isButton30Pressed = true;
      }
    } else if (channel == "Button45") {
      // ถ้ารับค่า "1" ให้มอเตอร์ขับเคลื่อนไปด้านหลัง
      if (payload == "1") {
        isButton45Pressed = true;
      }
    } else if (channel == "Button60") {
      // ถ้ารับค่า "1" ให้มอเตอร์ขับเคลื่อนไปด้านหลัง
      if (payload == "1") {
        isButton60Pressed = true;
      }
    }
  }
}

// ฟังก์ชันอัพเดตค่าจาก MPU6050 และส่งข้อมูลไปยัง Anto
void updateDegreeAngle() {
  // อ่านค่ามุมองศาจาก MPU6050
  float angle1 = getAngle(&mpu1);
  float angle2 = getAngle(&mpu2);
  
  // ส่งค่ามุมองศาไปยัง Anto MQTT
  anto.pub("Angle1", angle1);
  anto.pub("Angle2", angle2);
}

// ฟังก์ชันในการตั้งค่า Route
void setupRoutes() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/index.html", HTTP_GET, handleRoot);
  server.on("/assets/css/style.css", HTTP_GET, []() {
    handleFile("/assets/css/style.css");
  });
  server.on("/assets/js/main.js", HTTP_GET, []() {
    handleFile("/assets/js/main.js");
  });
}

// ฟังก์ชันในการจัดการหน้า Root
void handleRoot() {
  handleFile("/index.html");
}

// ฟังก์ชันในการให้ Content Type ของไฟล์
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

// ฟังก์ชันในการจัดเก็บไฟล์ด้วย SPIFFS
void handleFile(const String& fileName) {
  File file = LittleFS.open(fileName, "r");
  if (!file) {
    server.send(404, "text/plain", "File not found");
    return;
  }
  server.streamFile(file, getContentType(fileName));
  file.close();
}
