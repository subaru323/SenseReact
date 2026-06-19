/**
 * Car_Receiver.ino  （車体側 ESP32）
 *
 * ESP-NOWで受け取った左右の速度指令で、連続回転サーボを回す。
 * 差動駆動：left/right それぞれ -100〜100（%）。
 *
 * 【配線】
 *   左サーボ 信号 → GPIO13（左2個なら信号を分岐して両方へ）
 *   右サーボ 信号 → GPIO14
 *   サーボ VCC   → 電池+（4個なら必ず外部電池。1個テストならESP32の5V/VINでも可）
 *   サーボ GND   → 電池− かつ ESP32 GND（共通必須）
 *
 * 【安全】
 *   500msメッセージが来なければ自動停止（フェイルセーフ）。
 */

#include <esp_now.h>
#include <WiFi.h>
#include <ESP32Servo.h>

typedef struct {
  int16_t left;   // -100〜100（%）
  int16_t right;
} DriveCmd;

const int PIN_L = 13;
const int PIN_R = 14;

Servo servoL, servoR;

volatile int16_t curL = 0, curR = 0;
volatile unsigned long lastRecv = 0;

// 速度(%) → 連続回転サーボのマイクロ秒
// READMEのサーボ値に合わせる：停止1500 / 全開2500（逆転は対称で500）
int toUs(int speed) {
  speed = constrain(speed, -100, 100);
  return 1500 + speed * 10;  // -100→500, 0→1500, 100→2500
}

void onRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  if (len == sizeof(DriveCmd)) {
    DriveCmd c;
    memcpy(&c, data, sizeof(c));
    curL = c.left;
    curR = c.right;
    lastRecv = millis();
  }
}

void setup() {
  Serial.begin(115200);
  delay(300);

  servoL.attach(PIN_L, 500, 2500);   // 500〜2500μs（READMEの全開2500に対応）
  servoR.attach(PIN_R, 500, 2500);
  servoL.writeMicroseconds(1500);  // 停止
  servoR.writeMicroseconds(1500);

  WiFi.mode(WIFI_STA);
  Serial.print("車体 MAC: ");
  Serial.println(WiFi.macAddress());
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW 初期化失敗");
    return;
  }
  esp_now_register_recv_cb(onRecv);
  Serial.println("車体 受信待ち...");
}

void loop() {
  if (millis() - lastRecv > 500) {
    // 信号途絶 → 安全停止
    servoL.writeMicroseconds(1500);
    servoR.writeMicroseconds(1500);
  } else {
    servoL.writeMicroseconds(toUs(curL));
    servoR.writeMicroseconds(toUs(curR));
  }
  delay(20);
}
