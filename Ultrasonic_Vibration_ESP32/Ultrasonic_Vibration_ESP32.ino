/**
 * Ultrasonic_Vibration_ESP32.ino  （ESP32・単体完結版）
 *
 * HC-SR04で距離を測り、近いほど振動モーターを強く回す。
 * 無線は使わず、同一ESP32内で完結する察知→フィードバックループ。
 *
 * 【配線】
 *   HC-SR04 VCC  → 3.3V（3.3V駆動で動作確認済み・分圧不要）
 *   HC-SR04 GND  → GND
 *   HC-SR04 Trig → GPIO4
 *   HC-SR04 Echo → GPIO5
 *   振動モーター  → GPIO2(PWM) → 1kΩ → 2SC1815(ベース) → モーター
 *                   （GPIOで直接モーターを駆動できないためトランジスタ駆動）
 */

const int TRIG_PIN = 4;
const int ECHO_PIN = 5;
const int VIB_PIN  = 2;

const float DIST_MIN_CM = 5.0;    // これ以下は振動最大
const float DIST_MAX_CM = 100.0;  // これ以上は振動なし

void setup() {
  Serial.begin(115200);
  delay(300);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(VIB_PIN, OUTPUT);
  digitalWrite(TRIG_PIN, LOW);
  analogWrite(VIB_PIN, 0);

  Serial.println("超音波→振動 連携開始...");
}

void loop() {
  // 10usのトリガーパルスを送る
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 60000);

  int vibPower = 0;
  if (duration > 0) {
    float distanceCm = duration * 0.0343 / 2.0;
    // 近いほど強く：DIST_MIN_CM以下で255、DIST_MAX_CM以上で0
    vibPower = constrain((int)map((long)distanceCm, (long)DIST_MIN_CM, (long)DIST_MAX_CM, 255, 0), 0, 255);

    Serial.print("距離: ");
    Serial.print(distanceCm, 1);
    Serial.print(" cm  振動: ");
    Serial.println(vibPower);
  } else {
    Serial.println("応答なし（範囲外 or 未検出）→ 振動停止");
  }

  analogWrite(VIB_PIN, vibPower);
  delay(100);
}
