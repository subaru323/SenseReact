/**
 * Car_Max_Speed_Test.ino  （車体側 ESP32・電圧変更による速度検証用）
 *
 * 無線(ESP-NOW)を使わず、車体ESP32単独で4輪を常時最大速度・前進で回し続ける。
 * 電池の本数・種類を繋ぎ変えながら、速度の変化を目視で比較するためのテスト。
 *
 * 【配線（実機確認済み・Car_Receiverと同一）】
 *   GPIO13 → 後輪左（左グループ）
 *   GPIO26 → 前輪左（左グループ）
 *   GPIO14 → 後輪右（右グループ）
 *   GPIO25 → 前輪右（右グループ）
 *   サーボ VCC → 外部電池+（テスト対象。ここを繋ぎ変える）
 *   サーボ GND → 電池− かつ ESP32 GND（共通必須）
 *
 * ★注意：GeekServo 9G 360°の動作電圧上限は6V（乾電池4本相当）。
 *         それ以上の電圧は定格超過につき自己責任・短時間のみでご確認ください。
 */

#include <ESP32Servo.h>

const int PIN_RL = 13;   // 後輪左
const int PIN_FL = 26;   // 前輪左
const int PIN_RR = 14;   // 後輪右
const int PIN_FR = 25;   // 前輪右

Servo servoFL, servoRL, servoFR, servoRR;

int toUs(int speed) {
  speed = constrain(speed, -100, 100);
  return 1500 + speed * 11;  // -100→400, 0→1500, 100→2600（拡張レンジ）
}

void setup() {
  Serial.begin(115200);
  delay(300);

  servoFL.attach(PIN_FL, 400, 2600);
  servoRL.attach(PIN_RL, 400, 2600);
  servoFR.attach(PIN_FR, 400, 2600);
  servoRR.attach(PIN_RR, 400, 2600);

  Serial.println("========================================");
  Serial.println("  最大速度連続テスト（電圧を繋ぎ変えて比較）");
  Serial.println("========================================");

  // 最大速度・前進で固定
  servoFL.writeMicroseconds(toUs(100));
  servoRL.writeMicroseconds(toUs(100));
  servoFR.writeMicroseconds(toUs(-100));  // 右側は取り付け向きが逆
  servoRR.writeMicroseconds(toUs(-100));

  Serial.println("4輪、最大速度で回転中...");
}

void loop() {
  // 何もしない（常時最大速度を維持するだけ）
  delay(1000);
}
