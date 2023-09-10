#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

#define clockwise 2  // กำหนดขา Digital สำหรับ Motor
#define counterclockwise 3  // กำหนดขา Digital สำหรับ Motor
#define limitswf 8
#define limitswr 9

// อ่านค่าความเร็วมุมและค่าเอียงจาก MPU6050
int16_t ax, ay, az;
int16_t gx, gy, gz;

bool motorRunning = false; // ตัวแปรเพื่อตรวจสอบว่ามอเตอร์กำลังทำงานหรือไม่

float targetRoll = 0.0; // กำหนดค่ามุมเอียงที่คุณต้องการ

bool isButtonLimitSwFPressed = false;
bool isButtonLimitSwRPressed = false;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  
  pinMode(clockwise, OUTPUT);
  pinMode(counterclockwise, OUTPUT);
  pinMode(limitswf, INPUT_PULLUP);
  pinMode(limitswr, INPUT_PULLUP);
  
  mpu.initialize();
  if (mpu.testConnection()) {
    Serial.println("MPU6050 connection successful");
  } else {
    Serial.println("MPU6050 connection failed");
  }
}

void loop() {
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  float roll = atan2(ay, az) * RAD_TO_DEG;
  
  if (Serial.available() > 0) {
    char command = Serial.read();  // อ่านค่าที่ส่งมาจาก Serial Monitor
    if (command == '0') {
      // รีเซ็ทค่ามุมเอียง 0 องศา
      targetRoll = 0.0;
      //มอเตอร์หยุดหมุน
      MotorStop();
    } else if (command == '1') {
      // มอเตอร์หมุนตามเข็มนาฬิกา
      MotorClockwise();
    } else if (command == '2') {
      // มอเตอร์หมุนทวนเข็มนาฬิกา
      MotorCounterClockwise();
    } else if (command == '7') {
      // มอเตอร์หมุนตามเข็มนาฬิกา
      MotorClockwise();
      // เลือกค่ามุมเอียง 30 องศา
      targetRoll = 30.0;
      Serial.println("Selected Roll: 30 degrees");
    } else if (command == '8') {
      // มอเตอร์หมุนตามเข็มนาฬิกา
      MotorClockwise();
      // เลือกค่ามุมเอียง 45 องศา
      targetRoll = 45.0;
      Serial.println("Selected Roll: 45 degrees");
    } else if (command == '9') {
      // มอเตอร์หมุนตามเข็มนาฬิกา
      MotorClockwise();
      // เลือกค่ามุมเอียง 60 องศา
      targetRoll = 60.0;
      Serial.println("Selected Roll: 60 degrees");
    }
    
    if (motorRunning){
      while (true) {
        mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
        float roll = atan2(ay, az) * RAD_TO_DEG;
        
        // ถ้ามอเตอร์กำลังทำงาน แสดงค่า Roll บน Serial Monitor
        Serial.print("Roll (X-axis): ");
        Serial.println(roll);
        delay(100);
        if (roll >= targetRoll) {
          //มอเตอร์หยุดหมุน
          MotorStop();
          break;
        }
      }
    }
  }


  isButtonLimitSwFPressed = !digitalRead(limitswf);
  isButtonLimitSwRPressed = !digitalRead(limitswr);

  if (isButtonLimitSwFPressed) {
    MotorStop();
  } else if (isButtonLimitSwRPressed) {
    MotorStop();
  }
  
}

void MotorClockwise() {
  // มอเตอร์หมุนตามเข็มนาฬิกา
  digitalWrite(clockwise, HIGH);
  digitalWrite(counterclockwise, LOW);
  Serial.println("Motor clockwise");
  motorRunning = true;
}

void MotorCounterClockwise() {
  // มอเตอร์หมุนตามเข็มนาฬิกา
  digitalWrite(clockwise, LOW);
  digitalWrite(counterclockwise, HIGH);
  Serial.println("Motor counterclockwise");
  motorRunning = true;
}

void MotorStop() {
  //มอเตอร์หยุดหมุน
  digitalWrite(clockwise, LOW);
  digitalWrite(counterclockwise, LOW);
  Serial.println("Motor stop");
  motorRunning = false;
}
