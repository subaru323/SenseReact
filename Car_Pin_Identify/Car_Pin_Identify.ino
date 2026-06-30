/**
 * Car_Pin_Identify.ino  （ピン特定用・無線なし）
 *
 * 目的：
 *   どのGPIOにどのサーボ（モーター）が繋がっているか目視で特定する。
 *   ピンを小さい順に1本ずつ「動かす→止める」を繰り返す。
 *   シリアルモニタの表示と、実際に動いたモーターを見比べて
 *   「GPIO○○ ＝ どの位置のモーターか」をメモする。
 *
 * 【配線】Car_Receiver と同じ電源構成
 *   サーボ VCC(赤) → 外部電池 +（乾電池4本=6V）
 *   サーボ GND(黒) → 電池 − かつ ESP32 GND（共通必須）
 *   各サーボ 信号  → GPIO13 / 14 / 25 / 26 のいずれか（特定したい対象）
 *
 * 【使い方】
 *   1) USBのみ・サーボVCC外して書き込み（給電競合とブラウンアウト回避）
 *   2) 書き込み後に電池接続
 *   3) シリアルモニタ(115200)を開く
 *   4) 「GPIO14 動作中」と出た時に動いたモーターをメモ → 以降同様
 *
 * 13・14・15・25・26 の5本を小さい順にテスト。
 * （既存コードは13/14/25/26だが、15に配線した可能性も考慮して網羅）
 */

#include <ESP32Servo.h>

// 小さい順
const int PINS[] = {13, 14, 25, 26};
const int N = sizeof(PINS) / sizeof(PINS[0]);

const int RUN_SPEED = 60;     // 動作時の速度（%）
const int RUN_MS    = 2000;   // 動かす時間
const int STOP_MS   = 1500;   // 止める時間

Servo s;

int toUs(int speed) {
  speed = constrain(speed, -100, 100);
  return 1500 + speed * 10;   // -100→500, 0→1500, 100→2500
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("========================================");
  Serial.println("  ピン特定テスト（小さい順に1本ずつ）");
  Serial.println("  動いたモーターと GPIO番号 を対応メモせよ");
  Serial.println("========================================");
  delay(2000);
}

void loop() {
  for (int i = 0; i < N; i++) {
    int pin = PINS[i];

    Serial.print(">>> GPIO");
    Serial.print(pin);
    Serial.println(" 動作中（このモーターをメモ）");

    s.attach(pin, 500, 2500);
    s.writeMicroseconds(toUs(RUN_SPEED));
    delay(RUN_MS);

    s.writeMicroseconds(1500);   // 停止
    Serial.print("    GPIO");
    Serial.print(pin);
    Serial.println(" 停止");
    delay(200);
    s.detach();                  // 次のピンへ（チャンネル解放）

    delay(STOP_MS);
  }

  Serial.println("---- 1巡完了。3秒後に繰り返し ----");
  delay(3000);
}
