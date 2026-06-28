/**
 * Car_Servo_Diag.ino  （車体サーボ単体診断・無線なし）
 *
 * 目的：
 *   「サーボが動かない」原因を切り分けるための無線抜きテスト。
 *   ESP-NOWを一切使わず、4個のサーボを順番に動かして
 *   「配線」と「給電」だけを検証する。
 *   これで動けば → 無線/受信側の問題。
 *   これで動かなければ → 配線か電源の問題。
 *
 * 【配線】Car_Receiver と同じ
 *   GPIO13 → 左前(FL)   GPIO14 → 左後(RL)
 *   GPIO25 → 右前(FR)   GPIO26 → 右後(RR)
 *   サーボ赤(VCC) → 乾電池+（VINではなく電池から直接！）
 *   サーボ黒(GND) → 電池− かつ ESP32 GND（共通必須）
 *
 * 【確認手順】
 *   1) まずUSBだけ・サーボVCC外して書き込み、Serialに起動メッセージが出るか
 *   2) 次に電池つないでサーボVCC接続、4個が順に回るか
 *   3) 1個ずつ動かすので「どのサーボが死んでるか」も分かる
 */

#include <ESP32Servo.h>

const int PIN_FL = 13;
const int PIN_RL = 14;
const int PIN_FR = 25;
const int PIN_RR = 26;

Servo servoFL, servoRL, servoFR, servoRR;

int toUs(int speed) {
  speed = constrain(speed, -100, 100);
  return 1500 + speed * 10;  // -100→500, 0→1500, 100→2500
}

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("========================================");
  Serial.println("  車体サーボ診断（無線なし）");
  Serial.println("========================================");

  servoFL.attach(PIN_FL, 500, 2500);
  servoRL.attach(PIN_RL, 500, 2500);
  servoFR.attach(PIN_FR, 500, 2500);
  servoRR.attach(PIN_RR, 500, 2500);

  servoFL.writeMicroseconds(1500);
  servoRL.writeMicroseconds(1500);
  servoFR.writeMicroseconds(1500);
  servoRR.writeMicroseconds(1500);
  Serial.println("全サーボ停止（1500us）。3秒後に1個ずつテスト開始");
  delay(3000);
}

// 1個のサーボを「正転→停止→逆転→停止」させる
void testOne(const char *name, Servo &s) {
  Serial.print(name); Serial.println(" : 正転（前進）");
  s.writeMicroseconds(toUs(60));
  delay(1500);
  Serial.print(name); Serial.println(" : 停止");
  s.writeMicroseconds(1500);
  delay(800);
  Serial.print(name); Serial.println(" : 逆転（後退）");
  s.writeMicroseconds(toUs(-60));
  delay(1500);
  Serial.print(name); Serial.println(" : 停止");
  s.writeMicroseconds(1500);
  delay(800);
}

void loop() {
  Serial.println("---- 1個ずつテスト ----");
  testOne("FL(13)", servoFL);
  testOne("RL(14)", servoRL);
  testOne("FR(25)", servoFR);
  testOne("RR(26)", servoRR);

  Serial.println("---- 4個同時 全開前進 ----");
  servoFL.writeMicroseconds(toUs(80));
  servoRL.writeMicroseconds(toUs(80));
  servoFR.writeMicroseconds(toUs(80));
  servoRR.writeMicroseconds(toUs(80));
  delay(2000);

  servoFL.writeMicroseconds(1500);
  servoRL.writeMicroseconds(1500);
  servoFR.writeMicroseconds(1500);
  servoRR.writeMicroseconds(1500);
  Serial.println("全停止。2秒後にループ");
  delay(2000);
}
