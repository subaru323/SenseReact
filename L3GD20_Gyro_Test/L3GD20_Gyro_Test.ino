/**
 * L3GD20_Gyro_Test.ino
 *
 * L3GD20（3軸ジャイロ、ST社製）をI2Cで読む。指先の回転速度検出用。
 * 加速度・曲げは使わず、ジャイロの角速度(dps)だけを使う設計
 * （「回した分だけ曲がる」＝角速度に比例した旋回でよいため）。
 *
 * 【配線】
 *   VDD → 3.3V
 *   GND → GND
 *   SDA → D14
 *   SCL → D15
 *   CS  → 3.3V（I2Cモード選択・必須。浮かせるとSPIモード扱いで無応答になる）
 *   SA0 → GND(0x6A) または 3.3V(0x6B) ※どちらでもこのコードが自動判定
 *
 * ※MPU系（InvenSense）とはレジスタマップも読み出しの流儀も別物：
 *   ・複数バイト連続読み出し時はレジスタ番地の最上位ビットを立てる（自動インクリメント指示）
 *   ・出力はリトルエンディアン（下位バイトが先）
 */

#include <Wire.h>

const uint8_t REG_WHO_AM_I = 0x0F;
const uint8_t REG_CTRL1    = 0x20;
const uint8_t REG_CTRL4    = 0x23;
const uint8_t REG_OUT_X_L  = 0x28;

const float DPS_PER_LSB = 0.00875;  // フルスケール±245dps設定時の分解能

uint8_t gyroAddr = 0;
bool calibrated = false;                // 電源投入後、最初の1回だけ補正する
float zeroX = 0, zeroY = 0, zeroZ = 0;  // 起動時（静止状態）のオフセット

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

uint8_t findGyro() {
  for (uint8_t a = 0x6A; a <= 0x6B; a++) {
    uint8_t who = readReg(a, REG_WHO_AM_I);
    if (who == 0xD4 || who == 0xD7) return a;  // L3GD20=0xD4 / L3GD20H=0xD7
  }
  return 0;
}

bool readGyroRaw(uint8_t addr, int16_t &x, int16_t &y, int16_t &z) {
  Wire.beginTransmission(addr);
  Wire.write(REG_OUT_X_L | 0x80);  // 0x80=自動インクリメント有効（ST社の流儀）
  Wire.endTransmission(false);
  Wire.requestFrom((int)addr, 6);

  if (Wire.available() < 6) {
    while (Wire.available()) Wire.read();
    return false;
  }

  x = Wire.read() | (Wire.read() << 8);  // リトルエンディアン
  y = Wire.read() | (Wire.read() << 8);
  z = Wire.read() | (Wire.read() << 8);
  return true;
}

void calibrateZero(uint8_t addr) {
  long sx = 0, sy = 0, sz = 0;
  const int N = 50;
  int ok = 0;
  for (int i = 0; i < N; i++) {
    int16_t x, y, z;
    if (readGyroRaw(addr, x, y, z)) { sx += x; sy += y; sz += z; ok++; }
    delay(10);
  }
  if (ok == 0) return;
  zeroX = (sx / (float)ok) * DPS_PER_LSB;
  zeroY = (sy / (float)ok) * DPS_PER_LSB;
  zeroZ = (sz / (float)ok) * DPS_PER_LSB;

  Serial.print("ゼロ点補正 完了  X=");
  Serial.print(zeroX, 2);
  Serial.print(" Y=");
  Serial.print(zeroY, 2);
  Serial.print(" Z=");
  Serial.println(zeroZ, 2);
}

void setup() {
  Serial.begin(115200);
  unsigned long t = millis();
  while (!Serial && (millis() - t < 3000)) { ; }

  Wire.begin();
  Wire.setClock(100000);

  Serial.println("L3GD20を探索します（毎秒リトライ）...");
}

void loop() {
  if (gyroAddr == 0) {
    gyroAddr = findGyro();
    if (gyroAddr == 0) {
      Serial.println("見つかりません（配線・CS(3.3V)・SA0を確認）");
      delay(1000);
      return;
    }

    Serial.print("見つかった: 0x");
    Serial.println(gyroAddr, HEX);

    writeReg(gyroAddr, REG_CTRL1, 0x0F);  // 電源ON・XYZ有効・95Hz
    writeReg(gyroAddr, REG_CTRL4, 0x00);  // フルスケール±245dps
    delay(100);

    if (!calibrated) {
      // 電源投入後の最初の1回だけ補正する（配線が緩くて再検出を繰り返しても、
      // そのたびに補正し直すと手を動かした瞬間を拾ってゼロ点が狂うため）
      Serial.println("静止状態でゼロ点補正します（動かさないでください）...");
      calibrateZero(gyroAddr);
      calibrated = true;
    } else {
      Serial.println("再接続（前回のゼロ点補正を再利用）");
    }
    Serial.println("計測開始...");
  }

  static uint8_t failCount = 0;
  const uint8_t FAIL_LIMIT = 5;  // これだけ連続失敗したら断線とみなして再検出

  int16_t xr, yr, zr;
  if (readGyroRaw(gyroAddr, xr, yr, zr)) {
    failCount = 0;
    float gx = xr * DPS_PER_LSB - zeroX;
    float gy = yr * DPS_PER_LSB - zeroY;
    float gz = zr * DPS_PER_LSB - zeroZ;

    Serial.print("回転dps  X=");
    Serial.print(gx, 2);
    Serial.print(" Y=");
    Serial.print(gy, 2);
    Serial.print(" Z=");
    Serial.println(gz, 2);
  } else {
    failCount++;
    Serial.print("読み取り失敗（");
    Serial.print(failCount);
    Serial.print("/");
    Serial.print(FAIL_LIMIT);
    Serial.println("）配線がゆるい可能性");
    if (failCount >= FAIL_LIMIT) {
      Serial.println("連続失敗 → 再検出します");
      gyroAddr = 0;
      failCount = 0;
    }
  }

  delay(100);
}
