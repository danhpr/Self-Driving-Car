#include <Arduino.h>
#include <Servo.h>

// Định nghĩa các chân kết nối
#define TRIGGER_PIN 4    // Chân trigger của cảm biến siêu âm
#define ECHO_PIN 2       // Chân echo của cảm biến siêu âm
#define SERVO_PIN 11     // Chân điều khiển servo
#define IN1 7            // Chân điều khiển motor trái (A)
#define IN2 6            // Chân điều khiển motor trái (A)
#define IN3 9            // Chân điều khiển motor phải (B)
#define IN4 8            // Chân điều khiển motor phải (B)
#define EN_RIGHT 3       // Chân PWM cho motor phải
#define EN_LEFT 5        // Chân PWM cho motor trái

// Khởi tạo servo
Servo servo;

// Biến toàn cục
float distance = 0;            // Khoảng cách đo được từ cảm biến siêu âm
float left_distance = 0;       // Khoảng cách bên trái
float right_distance = 0;      // Khoảng cách bên phải
unsigned long duration_us = 0; // Thời gian phản hồi từ cảm biến siêu âm

// Hằng số
const int TIME_DELAY = 500;      // Thời gian chờ (ms) cho các hành động
const int SPEED_FORWARD = 150;   // Tốc độ tiến thẳng
const int SPEED_TURN = 170;      // Tốc độ khi quay
const float DISTANCE_LIMIT = 15.0; // Ngưỡng khoảng cách để phát hiện vật cản
const int SLOW_SPEED = 100;      // Tốc độ chậm khi gần vật cản

void setup() {
  // Cấu hình các chân cho motor
  pinMode(IN1, OUTPUT);      // Chân điều khiển motor trái (A)
  pinMode(IN2, OUTPUT);      // Chân điều khiển motor trái (A)
  pinMode(IN3, OUTPUT);      // Chân điều khiển motor phải (B)
  pinMode(IN4, OUTPUT);      // Chân điều khiển motor phải (B)
  pinMode(EN_LEFT, OUTPUT);  // Chân PWM motor trái
  pinMode(EN_RIGHT, OUTPUT); // Chân PWM motor phải

  // Cấu hình cảm biến siêu âm
  pinMode(TRIGGER_PIN, OUTPUT); // Chân trigger của HC-SR04
  pinMode(ECHO_PIN, INPUT);     // Chân echo của HC-SR04

  // Khởi tạo servo
  servo.attach(SERVO_PIN);      // Kết nối servo với chân điều khiển
  servo.write(90);             // Đặt servo nhìn thẳng về phía trước

  Serial.begin(9600);
}

// Đo khoảng cách bằng cảm biến siêu âm
float measure_distance() {
  digitalWrite(TRIGGER_PIN, LOW);   // Đặt trigger thấp để reset
  delayMicroseconds(2);             // Chờ 2 micro giây
  digitalWrite(TRIGGER_PIN, HIGH);  // Gửi xung trigger
  delayMicroseconds(10);            // Duy trì xung trong 10 micro giây
  digitalWrite(TRIGGER_PIN, LOW);   // Tắt trigger

  duration_us = pulseIn(ECHO_PIN, HIGH, 30000); // Đọc thời gian phản hồi, timeout 30ms
  if (duration_us == 0) return 999; // Trả về 999 nếu cảm biến lỗi hoặc ngoài phạm vi
  distance = duration_us * 0.0343 / 2.0; // Tính khoảng cách (cm)
  return distance;
}

// Điều khiển robot tiến thẳng với tốc độ tùy chỉnh
void go_forward(int speed) {
  analogWrite(EN_LEFT, speed);   // Đặt tốc độ motor trái
  analogWrite(EN_RIGHT, speed);  // Đặt tốc độ motor phải
  digitalWrite(IN1, HIGH);       // Motor trái quay thuận
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, HIGH);       // Motor phải quay thuận
  digitalWrite(IN4, LOW);
}

// Điều khiển robot lùi lại
void go_backward() {
  analogWrite(EN_LEFT, SPEED_FORWARD);  // Đặt tốc độ motor trái
  analogWrite(EN_RIGHT, SPEED_FORWARD); // Đặt tốc độ motor phải
  digitalWrite(IN1, LOW);        // Motor trái quay ngược
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);        // Motor phải quay ngược
  digitalWrite(IN4, HIGH);
}

// Điều khiển robot quay trái
void turn_left() {
  analogWrite(EN_LEFT, SPEED_TURN);  // Đặt tốc độ quay cho motor trái
  analogWrite(EN_RIGHT, SPEED_TURN); // Đặt tốc độ quay cho motor phải
  digitalWrite(IN1, LOW);            // Motor trái quay ngược
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, HIGH);           // Motor phải quay thuận
  digitalWrite(IN4, LOW);
}

// Điều khiển robot quay phải
void turn_right() {
  analogWrite(EN_LEFT, SPEED_TURN);  // Đặt tốc độ quay cho motor trái
  analogWrite(EN_RIGHT, SPEED_TURN); // Đặt tốc độ quay cho motor phải
  digitalWrite(IN1, HIGH);           // Motor trái quay thuận
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);            // Motor phải quay ngược
  digitalWrite(IN4, HIGH);
}

// Kiểm tra khoảng cách bên trái
float check_left() {
  servo.write(180);      // Xoay servo sang trái (180 độ)
  delay(TIME_DELAY);      // Chờ servo di chuyển
  float temp = measure_distance(); // Đo khoảng cách bên trái
  servo.write(90);       // Đưa servo về vị trí ban đầu
  delay(100);             // Chờ servo quay lại
  return temp;
}

// Kiểm tra khoảng cách bên phải
float check_right() {
  servo.write(0);        // Xoay servo sang phải (0 độ)
  delay(TIME_DELAY);      // Chờ servo di chuyển
  float temp = measure_distance(); // Đo khoảng cách bên phải
  servo.write(90);       // Đưa servo về vị trí ban đầu
  delay(100);             // Chờ servo quay lại
  return temp;
}

// Dừng toàn bộ motor
void stop() {
  analogWrite(EN_RIGHT, 0);  // Tắt PWM motor phải
  analogWrite(EN_LEFT, 0);   // Tắt PWM motor trái
  digitalWrite(IN1, LOW);    // Ngắt motor trái
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);    // Ngắt motor phải
  digitalWrite(IN4, LOW);
}

// Vòng lặp chính điều khiển robot
void loop() {
  // distance = measure_distance(); // Đo khoảng cách phía trước
  // if (distance >= DISTANCE_LIMIT) {
  //   go_forward(SPEED_FORWARD); // Tiến thẳng nếu không có vật cản
  // } else if (distance < 20 && distance >= DISTANCE_LIMIT) {
  //   go_forward(SLOW_SPEED); // Giảm tốc độ khi gần vật cản
  // } else {
  //   stop(); // Dừng lại khi gặp vật cản
  //   left_distance = check_left(); // Kiểm tra khoảng cách bên trái
  //   right_distance = check_right(); // Kiểm tra khoảng cách bên phải
  //   if (right_distance < DISTANCE_LIMIT && left_distance < DISTANCE_LIMIT) {
  //     go_backward(); // Lùi lại nếu cả hai bên đều có vật cản
  //     delay(TIME_DELAY);
  //     stop();
  //   } else if (right_distance >= left_distance) {
  //     turn_right(); // Quay phải nếu bên phải thoáng hơn
  //     delay(TIME_DELAY);
  //     stop();
  //   } else {
  //     turn_left(); // Quay trái nếu bên trái thoáng hơn
  //     delay(TIME_DELAY);
  //     stop();
  //   }
  // }
  
}
