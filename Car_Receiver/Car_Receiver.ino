/**
 * Car_Receiver.ino  （車体側 ESP32）
 *
 * ESP-NOWで受け取った左右の速度指令で、連続回転サーボ4個を回す。
 * 差動駆動：left/right それぞれ -100〜100（%）。
 *
 * 旋回ロジック：
 *   前進   left=+N, right=+N
 *   後退   left=-N, right=-N
 *   右旋回 left=0,  right=+N（左輪停止・右輪のみ前進）
 *   左旋回 left=+N, right=0 （右輪停止・左輪のみ前進）
 *
 * 【配線（実機確認済み）】
 *   GPIO13 → 後輪左（左グループ）
 *   GPIO26 → 前輪左（左グループ）
 *   GPIO14 → 後輪右（右グループ）
 *   GPIO25 → 前輪右（右グループ）
 *   サーボ VCC → 外部電池+（必須。ESP32の5Vでは電流不足）
 *   サーボ GND → 電池− かつ ESP32 GND（共通必須）
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

const int PIN_RL = 13;   // 後輪左
const int PIN_FL = 26;   // 前輪左
const int PIN_RR = 14;   // 後輪右
const int PIN_FR = 25;   // 前輪右

Servo servoFL, servoRL, servoFR, servoRR;

volatile int16_t curL = 0, curR = 0;
volatile unsigned long lastRecv = 0;

int toUs(int speed) {
  speed = constrain(speed, -100, 100);
  return 1500 + speed * 10;  // -100→500, 0→1500, 100→2500
}

void stopAll() {
  servoFL.writeMicroseconds(1500);
  servoRL.writeMicroseconds(1500);
  servoFR.writeMicroseconds(1500);
  servoRR.writeMicroseconds(1500);
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

  servoFL.attach(PIN_FL, 500, 2500);
  servoRL.attach(PIN_RL, 500, 2500);
  servoFR.attach(PIN_FR, 500, 2500);
  servoRR.attach(PIN_RR, 500, 2500);
  stopAll();

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
    stopAll();
  } else {
    // 左側：正転、右側：逆転（取り付け向きが逆のため）
    servoFL.writeMicroseconds(toUs( curL));
    servoRL.writeMicroseconds(toUs( curL));
    servoFR.writeMicroseconds(toUs(-curR));
    servoRR.writeMicroseconds(toUs(-curR));
  }
  delay(20);
}
