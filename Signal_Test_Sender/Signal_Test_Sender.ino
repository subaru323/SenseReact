/**
 * Signal_Test_Sender.ino  （送信側 ESP32・固定信号送信テスト）
 *
 * DriveCmd{left:60, right:60} を0.2秒ごとにブロードキャスト。
 * Car_Signal_Test（車体側・電池単独動作）でモーター回転を確認するための送信機。
 */

#include <esp_now.h>
#include <WiFi.h>

typedef struct {
  int16_t left;
  int16_t right;
} DriveCmd;

uint8_t broadcastAddr[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
DriveCmd cmd = {60, 60};   // 固定速度信号

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

  Serial.println("固定信号送信開始（left=60, right=60）");
}

void loop() {
  esp_now_send(broadcastAddr, (const uint8_t *)&cmd, sizeof(cmd));
  Serial.print("送信: left=");
  Serial.print(cmd.left);
  Serial.print(" right=");
  Serial.println(cmd.right);
  delay(200);
}
