/**
 * I2C_Scanner.ino
 *
 * I2Cバス上の全アドレス(0x01〜0x7E)を総当たりでスキャンし、
 * 応答があったアドレスを一覧表示する。
 * MPU-9250デュアル接続（0x68/0x69）の配線切り分け用。
 *
 * 期待値：
 *   0x68 … ①手首（AD0=GND）
 *   0x69 … ②指先（AD0=VCC）
 *   何も出ない場合はSDA/SCL/GND配線そのものを疑う。
 */

#include <Wire.h>

void setup()
{
  Serial.begin(115200);
  unsigned long t = millis();
  while (!Serial && (millis() - t < 3000))
  {
    ;
  }

  Wire.begin();
  Wire.setClock(100000);

  Serial.println("I2Cスキャン開始...");

  // D13はHIGH,D12はLOWにしておくと、I2Cスキャン中にLEDが点灯するので便利
  pinMode(13, OUTPUT);
  pinMode(12, OUTPUT);
  digitalWrite(13, HIGH);
  digitalWrite(12, LOW);
}

void loop()
{
  Serial.println("----------------------------------------");
  int found = 0;
  for (uint8_t addr = 1; addr < 127; addr++)
  {
    Wire.beginTransmission(addr);
    uint8_t err = Wire.endTransmission();
    if (err == 0)
    {
      Serial.print("応答あり: 0x");
      if (addr < 16)
        Serial.print("0");
      Serial.println(addr, HEX);
      found++;
    }
  }

  if (found == 0)
  {
    Serial.println("応答なし（デバイスが1つも見つかりません → SDA/SCL/GND配線を確認）");
  }
  else
  {
    Serial.print(found);
    Serial.println("個のデバイスを検出");
  }

  delay(2000);
}
