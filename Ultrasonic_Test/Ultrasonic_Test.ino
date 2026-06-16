/**
 * Ultrasonic_Test.ino
 *
 * US-015 超音波センサー距離計測スケッチ
 *
 * 【回路】
 *   VCC  → 拡張ボード VOUT（5V）
 *   GND  → 拡張ボード GND
 *   Trig → 拡張ボード D08（直結）
 *   Echo → 1kΩ → D09 → 2kΩ → GND（分圧）
 */

const int TRIG_PIN    = 8;
const int ECHO_PIN    = 9;
const int SERIAL_BAUD = 115200;

void setup() {
  Serial.begin(SERIAL_BAUD);
  unsigned long startWait = millis();
  while (!Serial && (millis() - startWait < 3000)) { ; }

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  Serial.println("========================================");
  Serial.println("  US-015 超音波センサー 距離計測");
  Serial.println("========================================");
}

void loop() {
  // Echoピンの生の状態を確認
  Serial.print("Echo生値: ");
  Serial.print(digitalRead(ECHO_PIN));
  Serial.print(" | ");

  // Trigに10μsパルスを送る
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Echoのパルス幅を計測
  long duration = pulseIn(ECHO_PIN, HIGH, 60000);

  // 距離に変換（cm）
  if (duration == 0) {
    Serial.println("タイムアウト（Echoを検出できず）");
  } else {
    float distance = duration / 58.0;
    Serial.print("距離: ");
    Serial.print(distance, 1);
    Serial.println(" cm");
  }

  delay(200);
}
