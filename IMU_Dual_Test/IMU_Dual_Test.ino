/**
 * IMU_Dual_Test.ino
 *
 * MPU-9250を2個、同じI2Cバス(D14/D15)で読む。
 *   ①手首側 = 0x68（AD0→GND）
 *   ②指先側 = 0x69（AD0→3.3V）
 *
 * 【配線（共通）】
 *   VCC → 3.3V / GND → GND / SDA → D14 / SCL → D15
 *   ①AD0 → GND（0x68）　②AD0 → 3.3V（0x69）
 *
 * ※ライブラリ不要：I2Cレジスタを直接読む（SPRESENSEで確実に動かすため）。
 */

#include <Wire.h>

const uint8_t MPU1 = 0x68;   // ①手首側
const uint8_t MPU2 = 0x69;   // ②指先側
const int SERIAL_BAUD = 115200;

void writeReg(uint8_t addr, uint8_t reg, uint8_t val) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.write(val);
  Wire.endTransmission();
}

uint8_t readReg(uint8_t addr, uint8_t reg) {
  Wire.beginTransmission(addr);
  Wire.write(reg);
  Wire.endTransmission(false);
  Wire.requestFrom((int)addr, 1);
  return Wire.read();
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

  uint8_t b[14];
  for (int i = 0; i < 14; i++) b[i] = Wire.read();

  int16_t axr = (int16_t)((b[0]  << 8) | b[1]);
  int16_t ayr = (int16_t)((b[2]  << 8) | b[3]);
  int16_t azr = (int16_t)((b[4]  << 8) | b[5]);
  int16_t gxr = (int16_t)((b[8]  << 8) | b[9]);
  int16_t gyr = (int16_t)((b[10] << 8) | b[11]);
  int16_t gzr = (int16_t)((b[12] << 8) | b[13]);

  // 加速度 ±2g → 16384 LSB/g
  ax = axr / 16384.0; ay = ayr / 16384.0; az = azr / 16384.0;
  // ジャイロ ±250dps → 131 LSB/(deg/s)
  gx = gxr / 131.0;   gy = gyr / 131.0;   gz = gzr / 131.0;
  return true;
}

void checkWho(const char *label, uint8_t addr) {
  uint8_t who = readReg(addr, 0x75);
  Serial.print(label);
  Serial.print(" WHO_AM_I = 0x");
  Serial.println(who, HEX);
  if (who != 0x71 && who != 0x73) {
    Serial.print("⚠ ");
    Serial.print(label);
    Serial.println(" が見つからない？配線/AD0/アドレスを確認");
  }
}

void pAxis(const char *name, float v) {
  Serial.print(name);
  if (v >= 0) Serial.print("+");
  Serial.print(v, 2);
  Serial.print(" ");
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  unsigned long t = millis();
  while (!Serial && (millis() - t < 3000)) { ; }

  Wire.begin();
  Wire.setClock(100000);  // 100kHzに下げて安定化

  checkWho("①手首(0x68)", MPU1);
  checkWho("②指先(0x69)", MPU2);

  writeReg(MPU1, 0x6B, 0x00);  // PWR_MGMT_1=0 → スリープ解除
  writeReg(MPU2, 0x6B, 0x00);
  delay(100);

  Serial.println("========================================");
  Serial.println("  MPU-9250 デュアル動作テスト");
  Serial.println("========================================");
}

void loop() {
  float ax1, ay1, az1, gx1, gy1, gz1;
  float ax2, ay2, az2, gx2, gy2, gz2;

  bool ok1 = readMotion(MPU1, ax1, ay1, az1, gx1, gy1, gz1);
  bool ok2 = readMotion(MPU2, ax2, ay2, az2, gx2, gy2, gz2);

  if (ok1) {
    Serial.print("①手首 加速度g ");
    pAxis("X", ax1); pAxis("Y", ay1); pAxis("Z", az1);
    Serial.print("| 回転dps ");
    pAxis("X", gx1); pAxis("Y", gy1); pAxis("Z", gz1);
    Serial.println();
  } else {
    Serial.println("①手首 読み取り失敗（配線確認）");
  }

  if (ok2) {
    Serial.print("②指先 加速度g ");
    pAxis("X", ax2); pAxis("Y", ay2); pAxis("Z", az2);
    Serial.print("| 回転dps ");
    pAxis("X", gx2); pAxis("Y", gy2); pAxis("Z", gz2);
    Serial.println();
  } else {
    Serial.println("②指先 読み取り失敗（配線確認）");
  }

  Serial.println("----------------------------------------");
  delay(150);
}
