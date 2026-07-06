/**
 * I2C_Bus_Recovery.ino
 *
 * オシロ観測「SDA=0.4V張り付き・パルスなし」を受けたバス診断＋復旧。
 * 5秒ごとに A→B→C→D を1サイクル実行する。オシロを当てたまま観測すること。
 *
 *  [A] SDA/SCL のアイドル電位確認（digitalRead）
 *  [B] GPIOとして open-drain 矩形波を出す（SCL→SDAの順、各約2秒・約1kHz）
 *      → オシロでパルスが見えるか＝SPRESENSEがピンを動かせるかの確認
 *  [C] バスロック解除（SCLを9クロック→STOP条件生成）
 *  [D] SDAが解放されたら Wire で 0x68/0x69 をプローブ
 *
 * ※open-drain方式（LOW駆動 or 開放のみ）なので短絡があっても安全側。
 */

#include <Wire.h>

// open-drain 操作：HIGHはプルアップ任せ（INPUT）、LOWだけ能動駆動
void odRelease(uint8_t pin) { pinMode(pin, INPUT); }
void odLow(uint8_t pin)     { pinMode(pin, OUTPUT); digitalWrite(pin, LOW); }

const char *lv(int v) { return v ? "HIGH" : "LOW"; }

void pulseTest(const char *name, uint8_t pin, int cycles) {
  Serial.print("[B] ");
  Serial.print(name);
  Serial.println(" に矩形波を出力中（約2秒・オシロで確認）...");
  for (int i = 0; i < cycles; i++) {
    odLow(pin);
    delayMicroseconds(500);
    odRelease(pin);
    delayMicroseconds(500);
  }
  odRelease(pin);
}

void setup() {
  Serial.begin(115200);
  unsigned long t = millis();
  while (!Serial && (millis() - t < 3000)) { ; }
  Serial.println("========================================");
  Serial.println("  I2C バス診断＋ロック解除");
  Serial.println("========================================");
}

void loop() {
  // --- [A] アイドル電位 ---
  odRelease(PIN_D14);
  odRelease(PIN_D15);
  delay(20);
  int sda = digitalRead(PIN_D14);
  int scl = digitalRead(PIN_D15);
  Serial.print("[A] SDA=");
  Serial.print(lv(sda));
  Serial.print("  SCL=");
  Serial.println(lv(scl));

  // --- [B] パルス試験（オシロで見る） ---
  pulseTest("SCL(D15)", PIN_D15, 2000);
  pulseTest("SDA(D14)", PIN_D14, 2000);
  Serial.println("    → SCLに波形が出てSDAに出ない場合＝SDAは外部に握られている");
  Serial.println("    → 両方出ない場合＝SPRESENSE側/基板の問題");

  // --- [C] バスロック解除（9クロック法＋STOP） ---
  Serial.println("[C] ロック解除を試行（SCL 9クロック→STOP）");
  for (int i = 0; i < 9 && digitalRead(PIN_D14) == LOW; i++) {
    odLow(PIN_D15);
    delayMicroseconds(50);
    odRelease(PIN_D15);
    delayMicroseconds(50);
  }
  // STOP条件：SCL解放中に SDA を LOW→解放（立ち上げ）
  odLow(PIN_D14);
  delayMicroseconds(50);
  odRelease(PIN_D15);
  delayMicroseconds(50);
  odRelease(PIN_D14);
  delay(20);

  sda = digitalRead(PIN_D14);
  Serial.print("    解除後 SDA=");
  Serial.println(lv(sda));

  // --- [D] SDAが生きていればI2Cプローブ ---
  if (sda == HIGH) {
    Serial.println("[D] SDA解放を確認 → I2Cプローブ実行");
    Wire.begin();
    Wire.setClock(100000);
    for (uint8_t addr = 0x68; addr <= 0x69; addr++) {
      Wire.beginTransmission(addr);
      Wire.write(0x75);
      uint8_t code = Wire.endTransmission();
      Serial.print("    0x");
      Serial.print(addr, HEX);
      Serial.print(" → 返り値 ");
      Serial.print(code);
      Serial.println(code == 0 ? " ★応答あり！" : "");
    }
    Wire.end();
  } else {
    Serial.println("[D] SDAがLOWのまま → プローブ不能（外部要因が握り続けている）");
  }

  Serial.println("---- 5秒後に再実行 ----");
  delay(5000);
}
