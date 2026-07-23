/**
 * Bend_Gyro_ESP_Sender.ino  （送信側 ESP32・単体完結版）
 *
 * 曲げセンサー（スロットル）とL3GD20ジャイロ（ステア）をESP32単体で読み取り、
 * ESP-NOWで車体（Car_Receiver）へDriveCmdを送信する。
 * SPRESENSE・UARTブリッジは不要（この車体制御チェーンから独立）。
 *
 * 【配線】
 *   曲げセンサー：3.3V → センサー → GPIO34(ADC1) → 10kΩ → GND
 *   L3GD20      ：VDD→3.3V, GND→GND, SDA→GPIO21, SCL→GPIO22,
 *                 CS→3.3V(I2Cモード固定), SA0→GNDまたは3.3V(自動判定)
 *
 * 【操作ロジック】
 *   throttle = 曲げ量（-100〜+100、まっすぐ=0。値が上がる方向の曲げ=前進、下がる方向の曲げ=後退）
 *   steer    = ジャイロZ軸回転速度（-100〜+100、回した分だけ）
 *   left  = throttle - max(0,  steer)   … 右に回すと左輪が減速
 *   right = throttle - max(0, -steer)   … 左に回すと右輪が減速
 *
 * 【曲げセンサー 実測校正値（2026-07-09 ESP32・3.3V回路）】
 *   値1900 → throttle +100%
 *   値1400 → throttle    0%（まっすぐ）
 *   値 400 → throttle -100%
 *
 * 【要調整】
 *   MAX_DPS … ステアの効き具合。実際に回して感触を見ながら調整すること
 */

#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>

// ===================== ESP-NOW =====================
typedef struct {
  int16_t left;
  int16_t right;
} DriveCmd;

uint8_t broadcastAddr[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
DriveCmd cmd = {0, 0};

// ===================== 曲げセンサー（スロットル・双方向） =====================
// 実測校正値：値1900=+100% / 値1400=0%（まっすぐ） / 値400=-100%
const int BEND_PIN = 34;
const int BEND_POS_MAX_DIFF = 500;   // 1900-1400（プラス方向の曲げ幅）
const int BEND_NEG_MAX_DIFF = 1000;  // 1400-400 （マイナス方向の曲げ幅）
int flatValue = 0;

// ===================== L3GD20（ジャイロ・ステア） =====================
const uint8_t REG_WHO_AM_I = 0x0F;
const uint8_t REG_CTRL1    = 0x20;
const uint8_t REG_CTRL4    = 0x23;
const uint8_t REG_OUT_X_L  = 0x28;
const float DPS_PER_LSB = 0.00875;
const float MAX_DPS     = 50.0;   // ★要調整：この回転速度でステア100%とする（感度2倍に変更済み）

uint8_t gyroAddr = 0;
bool gyroCalibrated = false;
float zeroX = 0, zeroY = 0, zeroZ = 0;
float lastGX = 0, lastGY = 0, lastGZ = 0;  // 直近のジャイロ値（回転ロジック検証用の表示）
uint8_t gyroFailCount = 0;
const uint8_t GYRO_FAIL_LIMIT = 5;

// ステアを滑らかにする（1ループあたりの変化量を制限）
int smoothedSteer = 0;
const int STEER_STEP = 4;  // ★要調整：大きいほど反応が速いが荒くなる

int smoothSteer(int target) {
  if (target > smoothedSteer)      smoothedSteer = min(target, smoothedSteer + STEER_STEP);
  else if (target < smoothedSteer) smoothedSteer = max(target, smoothedSteer - STEER_STEP);
  return smoothedSteer;
}

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
    if (who == 0xD4 || who == 0xD7) return a;
  }
  return 0;
}

bool readGyroRaw(uint8_t addr, int16_t &x, int16_t &y, int16_t &z) {
  Wire.beginTransmission(addr);
  Wire.write(REG_OUT_X_L | 0x80);  // 自動インクリメント（ST社の流儀）
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

void calibrateGyroZero(uint8_t addr) {
  long sx = 0, sy = 0, sz = 0;
  int ok = 0;
  const int N = 50;
  for (int i = 0; i < N; i++) {
    int16_t x, y, z;
    if (readGyroRaw(addr, x, y, z)) { sx += x; sy += y; sz += z; ok++; }
    delay(10);
  }
  if (ok == 0) return;
  zeroX = (sx / (float)ok) * DPS_PER_LSB;
  zeroY = (sy / (float)ok) * DPS_PER_LSB;
  zeroZ = (sz / (float)ok) * DPS_PER_LSB;
  Serial.print("ジャイロ ゼロ点補正 完了 Z=");
  Serial.println(zeroZ, 2);
}

// ジャイロが未検出/再検出中でも呼び出し可能。取得できなければ0を返す
int readSteer() {
  if (gyroAddr == 0) {
    gyroAddr = findGyro();
    if (gyroAddr == 0) return 0;

    Serial.print("L3GD20 見つかった: 0x");
    Serial.println(gyroAddr, HEX);
    writeReg(gyroAddr, REG_CTRL1, 0x0F);
    writeReg(gyroAddr, REG_CTRL4, 0x00);
    delay(100);

    if (!gyroCalibrated) {
      Serial.println("静止状態でジャイロのゼロ点補正します...");
      calibrateGyroZero(gyroAddr);
      gyroCalibrated = true;
    }
  }

  int16_t xr, yr, zr;
  if (readGyroRaw(gyroAddr, xr, yr, zr)) {
    gyroFailCount = 0;
    lastGX = xr * DPS_PER_LSB - zeroX;
    lastGY = yr * DPS_PER_LSB - zeroY;
    lastGZ = zr * DPS_PER_LSB - zeroZ;
    return constrain((int)map((long)(lastGZ * 100), (long)(-MAX_DPS * 100), (long)(MAX_DPS * 100), -100, 100), -100, 100);
  } else {
    gyroFailCount++;
    if (gyroFailCount >= GYRO_FAIL_LIMIT) {
      gyroAddr = 0;
      gyroFailCount = 0;
    }
    return 0;
  }
}

// ===================== setup / loop =====================
void setup() {
  Serial.begin(115200);
  delay(300);

  flatValue = analogRead(BEND_PIN);
  Serial.print("曲げセンサー 基準値（まっすぐ）= ");
  Serial.println(flatValue);

  Wire.begin(21, 22);  // SDA=21, SCL=22
  Wire.setClock(100000);

  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW 初期化失敗");
    return;
  }
  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, broadcastAddr, 6);
  peer.channel = 0;
  peer.encrypt = false;
  esp_now_add_peer(&peer);

  Serial.println("送信開始（曲げ→スロットル、ジャイロ→ステア）");
}

void loop() {
  // ---- 曲げセンサー：スロットル（値が上がる方向=+、下がる方向=-） ----
  int raw = analogRead(BEND_PIN);
  int throttle;
  if (raw >= flatValue) {
    throttle = constrain(map(raw - flatValue, 0, BEND_POS_MAX_DIFF, 0, 100), 0, 100);
  } else {
    throttle = constrain(map(flatValue - raw, 0, BEND_NEG_MAX_DIFF, 0, -100), -100, 0);
  }

  // ---- ジャイロ：ステア（急変させず小刻みに近づける） ----
  int steer = smoothSteer(readSteer());

  // ---- 合成 ----
  cmd.left  = constrain(throttle - max(0, steer), -100, 100);
  cmd.right = constrain(throttle - max(0, -steer), -100, 100);

  esp_now_send(broadcastAddr, (const uint8_t *)&cmd, sizeof(cmd));

  Serial.print("値："); Serial.print(raw);
  Serial.print("  曲げ度："); Serial.print(throttle);
  Serial.print("%");
  Serial.print("  X："); Serial.print(lastGX, 1);
  Serial.print("  Y："); Serial.print(lastGY, 1);
  Serial.print("  Z："); Serial.print(lastGZ, 1);
  Serial.print("  | steer="); Serial.print(steer);
  Serial.print(" → L="); Serial.print(cmd.left);
  Serial.print(" R="); Serial.println(cmd.right);

  delay(30);  // ★短縮（元100ms）：センサー読取・送信・ステア追従が約3倍速くなる
}
