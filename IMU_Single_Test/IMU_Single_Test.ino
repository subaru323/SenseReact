/**
 * IMU_Single_Test.ino
 *
 * MPU-9250を1個だけ接続した状態でのテスト。
 * 1秒ごとにアドレスを再スキャンするため、実行中に配線を繋ぎ変えても
 * SPRESENSEをリセットせずにそのまま検出できる。
 *
 * 【配線】
 *   VCC   → 3.3V
 *   GND   → GND
 *   SDA   → D14
 *   SCL   → D15
 *   AD0   → GND(0x68) または 3.3V(0x69)
 *   CS    → 3.3V
 *   FSYNC → GND
 */

#include <Wire.h>

uint8_t readReg(uint8_t addr, uint8_t reg) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom((int)addr, 1);
  return Wire.read();
}

void writeReg(uint8_t addr, uint8_t reg, uint8_t val) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

bool readMotion(uint8_t addr, float &ax, float &ay, float &az, float &gx, float &gy, float &gz) {
  Wire.beginTransmission(addr);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom((int)addr, 14);

  if (Wire.available() < 14) {
    while (Wire.available()) Wire.read();
    return false;
  }

  int16_t axr = (Wire.read() << 8) | Wire.read();
  int16_t ayr = (Wire.read() << 8) | Wire.read();
  int16_t azr = (Wire.read() << 8) | Wire.read();
  Wire.read(); Wire.read();  // 温度は読み捨て
  int16_t gxr = (Wire.read() << 8) | Wire.read();
  int16_t gyr = (Wire.read() << 8) | Wire.read();
  int16_t gzr = (Wire.read() << 8) | Wire.read();

  ax = axr / 16384.0;
  ay = ayr / 16384.0;
  az = azr / 16384.0;
  gx = gxr / 131.0;
  gy = gyr / 131.0;
  gz = gzr / 131.0;
  return true;
}

uint8_t scanForMPU() {
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) return addr;
  }
  return 0;
}

void setup() {
  Serial.begin(115200);
  unsigned long t = millis();
  while (!Serial && (millis() - t < 3000)) { ; }

  Wire.begin();
  Wire.setClock(10000);

  Serial.println("1秒ごとにスキャンします。配線を繋ぎ変えてもリセット不要です。");
}

void loop() {
  uint8_t addr = scanForMPU();

  if (addr == 0) {
    Serial.println("見つかりません（配線を確認）");
  } else {
    Serial.print("見つかった: 0x");
    Serial.println(addr, HEX);

    uint8_t who = readReg(addr, 0x75);
    Serial.print("  WHO_AM_I = 0x");
    Serial.println(who, HEX);

    writeReg(addr, 0x6B, 0x00);  // スリープ解除
    delay(50);

    float ax, ay, az, gx, gy, gz;
    if (readMotion(addr, ax, ay, az, gx, gy, gz)) {
      Serial.print("  加速度 X=");
      Serial.print(ax, 2);
      Serial.print(" Y=");
      Serial.print(ay, 2);
      Serial.print(" Z=");
      Serial.print(az, 2);
      Serial.print("  ジャイロ X=");
      Serial.print(gx, 1);
      Serial.print(" Y=");
      Serial.print(gy, 1);
      Serial.print(" Z=");
      Serial.println(gz, 1);
    } else {
      Serial.println("  データ読み取り失敗");
    }
  }

  delay(1000);
}
