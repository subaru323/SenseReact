/**
 * Car_Signal_Test.ino  （車体側 ESP32・信号受信でモーター単体テスト）
 *
 * ESP-NOWでDriveCmdを受信したら、後輪左（GPIO13）のみを回転させる。
 * USBを繋がず、外部電池（VN給電）だけで動くかを確認するためのテスト。
 *
 * 送信側は Signal_Test_Sender.ino を使用。
 *
 * 【配線】
 *   GPIO13 → 後輪左
 *   サーボ VCC → 外部電池+
 *   サーボ GND → 電池− かつ ESP32 GND（共通必須）
 *   ESP32給電  → VN(VIN)ピン ← 電池+
 *
 * 【安全】
 *   500ms メッセージが来なければ自動停止（フェイルセーフ）。
 */

#include <esp_now.h>
#include <WiFi.h>
#include <ESP32Servo.h>

typedef struct {
  int16_t left;   // -100〜100（%）
  int16_t right;
} DriveCmd;

const int PIN_RL = 13;   // 後輪左のみ使用（単体テスト）

Servo servoRL;

volatile int16_t curL = 0;
volatile unsigned long lastRecv = 0;

int toUs(int speed) {
  speed = constrain(speed, -100, 100);
  return 1500 + speed * 10;  // -100→500, 0→1500, 100→2500
}

void onRecv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  if (len == sizeof(DriveCmd)) {
    DriveCmd c;
    memcpy(&c, data, sizeof(c));
    curL = c.left;
    lastRecv = millis();
  }
}

void setup() {
  Serial.begin(115200);
  delay(300);

  servoRL.attach(PIN_RL, 500, 2500);
  servoRL.writeMicroseconds(1500);  // 停止

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW 初期化失敗");
    return;
  }
  esp_now_register_recv_cb(onRecv);
  Serial.println("車体 受信待ち（信号が来たら後輪左が回転）...");
}

void loop() {
  if (millis() - lastRecv > 500) {
    servoRL.writeMicroseconds(1500);
  } else {
    servoRL.writeMicroseconds(toUs(curL));
  }
  delay(20);
}
