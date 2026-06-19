/**
 * Drive_Test_Sender.ino  （テスト送信側 ESP32）
 *
 * 車体（Car_Receiver）の動作確認用。
 * 「前進 → 右旋回 → 左旋回 → 停止」を1.5秒ごとに繰り返し、
 * 左右の速度指令をESP-NOWでブロードキャストする。
 *
 * ※本番ではこの送信側を「SPRESENSE→ESP32ブリッジ」に置き換える。
 */

#include <esp_now.h>
#include <WiFi.h>

typedef struct {
  int16_t left;
  int16_t right;
} DriveCmd;

uint8_t broadcastAddr[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
DriveCmd cmd;

void setup() {
  Serial.begin(115200);
  delay(300);
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
  Serial.println("テスト送信開始");
}

void loop() {
  // 1.5秒ごとにフェーズが変わる
  int phase = (millis() / 1500) % 4;
  const char *name;
  switch (phase) {
    case 0: cmd.left =  60; cmd.right =  60; name = "前進";   break;
    case 1: cmd.left =  60; cmd.right = -60; name = "右旋回"; break;
    case 2: cmd.left = -60; cmd.right =  60; name = "左旋回"; break;
    default: cmd.left =  0; cmd.right =   0; name = "停止";   break;
  }

  esp_now_send(broadcastAddr, (const uint8_t *)&cmd, sizeof(cmd));
  Serial.print(name);
  Serial.print("  L=");
  Serial.print(cmd.left);
  Serial.print(" R=");
  Serial.println(cmd.right);

  delay(100);  // 100msごとに送信（フェイルセーフより速く）
}
