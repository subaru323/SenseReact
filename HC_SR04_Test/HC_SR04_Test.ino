/**
 * HC_SR04_Test.ino  （SPRESENSE版・分圧なし直結）
 *
 * 超音波距離センサー HC-SR04 の単体動作確認。
 * 既存のUltrasonic_Test/Sense_Coreと同じD08/D09を踏襲。
 *
 * 【配線】
 *   VCC  → 5V
 *   GND  → GND
 *   Trig → D08（出力）
 *   Echo → D09（入力・直結）
 */

const int TRIG_PIN = 8;   // D08
const int ECHO_PIN = 9;   // D09

void setup() {
  Serial.begin(115200);
  delay(300);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  digitalWrite(TRIG_PIN, LOW);

  Serial.println("HC-SR04 距離測定開始...");
}

void loop() {
  // 10usのトリガーパルスを送る
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Echoパルスの長さを測る（タイムアウト60ms）
  long duration = pulseIn(ECHO_PIN, HIGH, 60000);

  if (duration == 0) {
    Serial.println("応答なし（配線・対象物との距離を確認）");
  } else {
    float distanceCm = duration * 0.0343 / 2.0;
    Serial.print("距離: ");
    Serial.print(distanceCm, 1);
    Serial.println(" cm");
  }

  delay(200);
}
