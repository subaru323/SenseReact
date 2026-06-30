/**
 * Car_Auto_Drive.ino  （車体側 ESP32・単独自走テスト）
 *
 * 無線(ESP-NOW)を使わず、車体ESP32単独で
 * 「前進5秒 → 後退5秒 → 右旋回5秒 → 左旋回5秒」を自動で繰り返す。
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
 *   サーボ VCC → 外部電池+（乾電池4本=6V）
 *   サーボ GND → 電池− かつ ESP32 GND（共通必須）
 *   ESP32給電  → VN(VIN)ピン ← 電池+
 */

#include <ESP32Servo.h>

const int PIN_RL = 13;   // 後輪左
const int PIN_FL = 26;   // 前輪左
const int PIN_RR = 14;   // 後輪右
const int PIN_FR = 25;   // 前輪右

const int SPEED        = 60;     // 走行速度（%）
const unsigned long PHASE_MS = 5000;  // 各動作の継続時間

Servo servoFL, servoRL, servoFR, servoRR;

int toUs(int speed) {
  speed = constrain(speed, -100, 100);
  return 1500 + speed * 10;  // -100→500, 0→1500, 100→2500
}

void drive(int left, int right) {
  servoFL.writeMicroseconds(toUs(left));
  servoRL.writeMicroseconds(toUs(left));
  servoFR.writeMicroseconds(toUs(right));
  servoRR.writeMicroseconds(toUs(right));
}

void setup() {
  Serial.begin(115200);
  delay(300);

  servoFL.attach(PIN_FL, 500, 2500);
  servoRL.attach(PIN_RL, 500, 2500);
  servoFR.attach(PIN_FR, 500, 2500);
  servoRR.attach(PIN_RR, 500, 2500);
  drive(0, 0);

  Serial.println("========================================");
  Serial.println("  自走テスト（無線なし）");
  Serial.println("  前進5s→後退5s→右旋回5s→左旋回5s を繰り返す");
  Serial.println("========================================");
}

void loop() {
  // 0:前進 1:後退 2:右旋回 3:左旋回 を5秒ごとに切り替え
  int phase = (millis() / PHASE_MS) % 4;
  const char *name;
  int left, right;
  switch (phase) {
    case 0: left =  SPEED; right =  SPEED; name = "前進";   break;
    case 1: left = -SPEED; right = -SPEED; name = "後退";   break;
    case 2: left =  0;     right =  SPEED; name = "右旋回"; break;
    default: left = SPEED; right =  0;     name = "左旋回"; break;
  }

  drive(left, right);

  static int lastPhase = -1;
  if (phase != lastPhase) {
    lastPhase = phase;
    Serial.print(name);
    Serial.print("  L=");
    Serial.print(left);
    Serial.print(" R=");
    Serial.println(right);
  }

  delay(50);
}
