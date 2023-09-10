#define clockwise 2  // กำหนดขา Digital สำหรับ Motor
#define counterclockwise 3  // กำหนดขา Digital สำหรับ Motor

void setup() {
  Serial.begin(9600);  // เปิด Serial Communication ที่อัตรา 9600 bps
  pinMode(clockwise, OUTPUT);
  pinMode(counterclockwise, OUTPUT);
}

void loop() {
  if (Serial.available() > 0) {
    char command = Serial.read();  // อ่านค่าที่ส่งมาจาก Serial Monitor
    
    if (command == '1') {
      // หมุนตามเข็มนาฬิกา
      digitalWrite(clockwise, HIGH);
      digitalWrite(counterclockwise, LOW);
      Serial.println("Motor clockwise");
    } else if (command == '2') {
      // หมุนทวนเข็มนาฬิกา
      digitalWrite(clockwise, LOW);
      digitalWrite(counterclockwise, HIGH);
      Serial.println("Motor counterclockwise");
    } else if (command == '0') {
      digitalWrite(clockwise, LOW);
      digitalWrite(counterclockwise, LOW);
      Serial.println("Motor stop");
    }
  }
}
