/**
 * I2C_Deep_Diag.ino
 *
 * I2C不通の原因を「バス異常」か「デバイス無応答」かに確定させる診断。
 * SPRESENSEコアのWire.cpp実装（endTransmissionの返り値仕様）に基づく。
 *
 *   返り値 0 = ACK（デバイスあり）
 *   返り値 2 = NACK（バスは正常・デバイスだけが無応答 → モジュール側）
 *   返り値 4 = バス異常（信号が出せない/詰まっている → SPRESENSE側/配線）
 *
 * さらにWire.begin()前にSDA/SCLの電位を直接読み、
 * プルアップ有無・バス張り付きも判定する。
 */

#include <Wire.h>

bool wireStarted = false;

const char *codeMeaning(uint8_t code) {
  switch (code) {
    case 0: return "ACK！デバイス応答あり";
    case 2: return "NACK（バス正常・デバイス無応答）";
    case 4: return "バス異常（信号レベルの問題）";
    default: return "その他";
  }
}

void probe(uint8_t addr) {
  // 1バイト実書き込み（WHO_AM_Iレジスタ番地を送るだけ）で確実にバスを叩く
  Wire.beginTransmission(addr);
  Wire.write(0x75);
  uint8_t code = Wire.endTransmission();

  Serial.print("  0x");
  Serial.print(addr, HEX);
  Serial.print(" → 返り値 ");
  Serial.print(code);
  Serial.print(" : ");
  Serial.println(codeMeaning(code));
}

void setup() {
  Serial.begin(115200);
  unsigned long t = millis();
  while (!Serial && (millis() - t < 3000)) { ; }

  Serial.println("========================================");
  Serial.println("  I2C 深掘り診断");
  Serial.println("========================================");

  // --- ステップ1：Wire開始前に生のピン電位を確認 ---
  pinMode(PIN_D14, INPUT);   // SDA
  pinMode(PIN_D15, INPUT);   // SCL
  delay(50);
  int sda = digitalRead(PIN_D14);
  int scl = digitalRead(PIN_D15);

  Serial.print("[1] アイドル時の電位  SDA(D14)=");
  Serial.print(sda ? "HIGH" : "LOW");
  Serial.print("  SCL(D15)=");
  Serial.println(scl ? "HIGH" : "LOW");

  if (sda && scl) {
    Serial.println("    → 両方HIGH：プルアップ正常・バスは空いている");
  } else {
    Serial.println("    ★異常：LOWの線がある＝短絡かバスロックか配線ミス");
    Serial.println("      （この状態では以降の通信は全て失敗します）");
  }

  // --- ステップ2：Wire初期化 ---
  Wire.begin();
  Wire.setClock(100000);
  wireStarted = true;
  Serial.println("[2] Wire.begin() 完了");
  Serial.println("    （↑この直前に『Failed to init I2C device』が出た場合は");
  Serial.println("      I2Cペリフェラル自体の初期化失敗＝SPRESENSE要再起動）");
}

void loop() {
  Serial.println("[3] 1バイト書き込みプローブ（0x68 / 0x69）");
  probe(0x68);
  probe(0x69);
  Serial.println("----------------------------------------");
  delay(1000);
}
