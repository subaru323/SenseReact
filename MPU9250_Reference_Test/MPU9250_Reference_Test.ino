/**
 * MPU9250_Reference_Test.ino
 *
 * 動作実績があるという参照コードをほぼそのまま使用。
 * 唯一の狙い：Wire.setClock()を一切呼ばない状態で反応が変わるかを確認する。
 */

#include <Wire.h>

const int MPU_ADDR = 0x68;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  delay(100);

  // MPU9250のスリープモードを解除
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B);  // PWR_MGMT_1 レジスタ
  Wire.write(0x00);  // 0を書き込んでスリープ解除
  byte error = Wire.endTransmission();

  if (error == 0) {
    Serial.println("スリープ解除成功。データ取得を開始します。");
  } else {
    Serial.println("通信エラー。配線を見直してください。");
  }
}

void loop() {
  // 加速度とジャイロの先頭レジスタ（0x3B）から読み出し開始を設定
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);
  Wire.endTransmission(false);

  // 14バイト（加速度6byte + 温度2byte + ジャイロ6byte）を要求
  Wire.requestFrom(MPU_ADDR, 14, true);

  if (Wire.available() == 14) {
    int16_t AcX = Wire.read() << 8 | Wire.read();
    int16_t AcY = Wire.read() << 8 | Wire.read();
    int16_t AcZ = Wire.read() << 8 | Wire.read();
    int16_t Tmp = Wire.read() << 8 | Wire.read();
    int16_t GyX = Wire.read() << 8 | Wire.read();
    int16_t GyY = Wire.read() << 8 | Wire.read();
    int16_t GyZ = Wire.read() << 8 | Wire.read();

    Serial.print("Acc X: "); Serial.print(AcX);
    Serial.print(" | Y: "); Serial.print(AcY);
    Serial.print(" | Z: "); Serial.print(AcZ);

    Serial.print(" || Gyro X: "); Serial.print(GyX);
    Serial.print(" | Y: "); Serial.print(GyY);
    Serial.print(" | Z: "); Serial.println(GyZ);
  } else {
    Serial.println("見つかりません（配線を確認）");
  }

  delay(50);
}
