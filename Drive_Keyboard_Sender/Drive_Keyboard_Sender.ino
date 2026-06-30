/**
 * Drive_Keyboard_Sender.ino  （送信側 ESP32・固定シーケンス操縦）
 *
 * 「前進5秒 → 後退5秒 → 右旋回5秒 → 左旋回5秒」を自動で繰り返し、
 * ESP-NOWで車体（Car_Receiver）へ送信する。
 *
 * 旋回ロジック（Car_Receiverの仕様に準拠）：
 *   前進   left=+N, right=+N
 *   後退   left=-N, right=-N
 *   右旋回 left=0,  right=+N（左輪停止・右輪のみ前進）
 *   左旋回 left=+N, right=0 （右輪停止・左輪のみ前進）
 */

#include <esp_now.h>
#include <WiFi.h>

typedef struct {
  int16_t left;
  int16_t right;
} DriveCmd;

const int SPEED        = 60;     // 走行速度（%）
const unsigned long PHASE_MS = 5000;  // 各動作の継続時間

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

  Serial.println("========================================");
  Serial.println("  固定シーケンス送信機");
  Serial.println("  前進5s→後退5s→右旋回5s→左旋回5s を繰り返す");
  Serial.println("========================================");
}

void loop() {
  // 0:前進 1:後退 2:右旋回 3:左旋回 を5秒ごとに切り替え
  int phase = (millis() / PHASE_MS) % 4;
  const char *name;
  switch (phase) {
    case 0: cmd.left =  SPEED; cmd.right =  SPEED; name = "前進";   break;
    case 1: cmd.left = -SPEED; cmd.right = -SPEED; name = "後退";   break;
    case 2: cmd.left =  0;     cmd.right =  SPEED; name = "右旋回"; break;
    default: cmd.left = SPEED; cmd.right =  0;     name = "左旋回"; break;
  }

  esp_now_send(broadcastAddr, (const uint8_t *)&cmd, sizeof(cmd));
  Serial.print(name);
  Serial.print("  L=");
  Serial.print(cmd.left);
  Serial.print(" R=");
  Serial.println(cmd.right);

  delay(100);  // 100msごとに送信（フェイルセーフ500msより速く）
}
