/**
 * GP2Y0E03_Test.ino  （ESP32・アナログ出力版の単体確認）
 *
 * 距離センサー GP2Y0E03（3ピン・アナログ版）の値取得確認。
 * まずは生のADC値と電圧だけを表示し、値が取れているかを見る。
 *
 * 【配線】
 *   VDD  → 3.3V
 *   GND  → GND
 *   Vout → GPIO34（ADC1）
 */

const int SENSOR_PIN = 34;

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println("GP2Y0E03 値確認開始...");
}

void loop() {
  int raw = analogRead(SENSOR_PIN);
  float voltage = raw * 3.3 / 4095.0;

  Serial.print("raw: ");
  Serial.print(raw);
  Serial.print("  電圧: ");
  Serial.print(voltage, 2);
  Serial.println(" V");

  delay(200);
}
