# SenseReact

テックシーカーハッカソン2026 出展作品

## これは何？

SenseReact（センスリアクト）は「**かざして感じ、かざして動かす**」前方感応型の身体拡張デバイス。
人が感じ取れない前方の情報をセンサーで「察知」し、振動で身体に伝え、手のポーズで外部マシンを「動かす」までを一連で実現する。

**動作フロー：**

1. **察知** … 超音波センサー・HDRカメラで、手をかざした先を捉える
2. **フィードバック** … 振動モーターで距離・気配を手首に伝える
3. **ポーズ検知** … 曲げセンサー・加速度センサーで手の構えを読み取る
4. **出力** … サーボ／ラジコンなど外部マシンが動く

## 進捗（2026-07-06）

**✅ 終わってる**
- カメラ：撮影（VGA）・動体検知
- 入力センサー：曲げ・超音波・9軸IMU（加速度・ジャイロ）
- 出力：連続回転サーボ4輪個別制御・振動モーター
- 超音波 → 振動（近いほど強く）= MVP前半
- 無線：ESP-NOW疎通・車体サーボを無線で制御
- 察知コア統合（超音波・振動・カメラ・IMU を1つに。腕が動いてる時の画面変化＝自分の動きを除外）
- SPRESENSE ↔ ESP32 UARTブリッジ（コード完成・疎通確認済み）
- ダッシュボード（Python中継サーバー＋ブラウザUI）
- 曲げセンサー → ESP-NOW → 車体4輪制御の一連フロー（コード完成）
- **実配線に基づくピン特定完了**（13=後輪左, 14=後輪右, 25=前輪右, 26=前輪左）。`Car_Receiver` の左右グループをこの配線に合わせて訂正済み
- 旋回ロジック確定：前進/後退は左右同方向、右旋回は左輪停止+右輪前進、左旋回は右輪停止+左輪前進
- 車体ESP32単独で動く自走テスト（`Car_Auto_Drive`）作成。無線なしで前進5s→後退5s→右旋回5s→左旋回5sを自動実行
- ESP-NOW疎通を実機再確認（Hello送受信・車体側でMAC付き連続受信OK）
- **車体ESP32の電源問題を解決**：乾電池→VN給電はWi-Fi使用時に不安定（電流スパイクでブラウンアウト）。**モバイルバッテリーUSB給電で安定動作を確認**。本番はこの構成
- ESP-NOW信号受信で単体モーター回転成功（`Car_Signal_Test` / `Signal_Test_Sender`）＝無線→駆動の最小構成を実証
- 指先用IMU②の故障診断完了（詳細は `docs/2026-07-06_作業報告.md`）

**⬜ 残タスク（優先度順）**

| 優先 | タスク | 状態 |
|---|---|---|
| 🔴 必須 | **拡張ボードI/O電圧ジャンパを5V→3.3Vへ**（IMU破損の再発防止。次のIMUを繋ぐ前に必ず） | 未対応 |
| 🔴 高 | 指先用IMUの交換調達（**MPU-6050/GY-521推奨**・レジスタ互換でコードほぼ無修正） | 手配待ち |
| 🔴 高 | MVP統合デモ動作確認（曲げ→無線→4輪が実際に動く） | 未完 |
| 🟡 中 | 指先IMU「指差し回転→旋回」ロジック（交換品到着まで①手首IMUで先行開発可） | 未着手 |
| 🟡 中 | `Car_Auto_Drive` 実機検証（4輪が訂正後のピンで正しく動くか） | 未検証 |
| 🟡 中 | ダッシュボード実機動作確認（bridge.py + Sense_Core 同時起動） | 未確認 |
| 🟡 中 | 音声テスト（Audio_Test をイヤホン繋いで確認） | 未確認 |
| 🟢 低 | 発表用デモ調整・見せ方整理 | 未着手 |
| ❌ 故障 | 指先用IMU②（MPU-9250）＝SDAパッド破損（5V I/O定格超過が原因とみられる）・交換一択 | 交換待ち |
| ❌ 断念 | 地磁気センサー（クローン品のためAK8963非搭載・実装不可） | 断念 |
| ❌ 保留 | GPS方位（屋内でFix不可） | 保留 |

## ファイル構成

```
テックシーカー2026/
├─ README.md              … このファイル（全体の説明・進捗・配線）
├─ PROJECT_CONTEXT.txt    … チームメンバーのAI向け文脈共有ドキュメント
├─ docs/                  … 補足資料・メモ
│
├─【SPRESENSE 装着側】
│  ├─ HDR_Camera_Capture/    … カメラ撮影確認（最初の動作確認用）
│  ├─ Camera_MotionDetect/   … カメラ動体検知（フレーム差分・察知の基礎）
│  ├─ Bend_Sensor_Test/      … 曲げセンサー単体テスト（キャリブ確認）
│  ├─ Ultrasonic_Test/       … 超音波距離計測（US-015）単体
│  ├─ Vibration_Test/        … 振動モーターPWM強さテスト
│  ├─ Ultrasonic_Vibration/  … 超音波×振動 統合（近いほど強く）
│  ├─ Servo_Test/            … サーボ単体スイープ
│  ├─ Bend_Servo_Control/    … 曲げ→サーボ速度連動（有線版）
│  ├─ MPU9250_Motion/        … 9軸IMU 加速度・ジャイロ読み取り
│  ├─ IMU_Mag_Test/          … IMU+地磁気テスト（地磁気は非搭載でスキップ）
│  ├─ IMU_Single_Test/       … IMU単体テスト（アドレス自動判定・毎秒再スキャン）
│  ├─ IMU_Dual_Test/         … IMU2個同時読み（0x68手首+0x69指先・交換品到着後に使用）
│  ├─ MPU9250_Reference_Test/… 参照コード移植版（診断切り分け用）
│  ├─ I2C_Scanner/           … I2C全アドレススキャン（診断用）
│  ├─ I2C_Deep_Diag/         … I2Cバス異常/デバイス無応答の切り分け（診断用）
│  ├─ I2C_Bus_Recovery/      … SDA/SCL電位・パルス試験・バスロック解除（オシロ診断用）
│  ├─ Sense_Core/            … ★察知コア統合（超音波・振動・カメラ・IMU）
│  ├─ Bridge_SPRESENSE/      … ★曲げ値をUARTでESP32へ送信（無線操縦の入口）
│  ├─ Audio_Test/            … ビープ音量テスト
│  └─ Nav_Compass_GPS/       … GPS・磁気テスト（保留）
│
├─【ESP32 無線系】
│  ├─ ESP32_Blink/           … ESP32動作確認（Lチカ）
│  ├─ ESPNow_Sender/         … ESP-NOW送信テスト（疎通確認用）
│  ├─ ESPNow_Receiver/       … ESP-NOW受信テスト（疎通確認用）
│  ├─ Bridge_ESP_Sender/     … ★UART受信→ESP-NOW中継（送信側ESP32）
│  ├─ Drive_Test_Sender/     … 車体テスト送信（スピンターン方式・旧仕様）
│  ├─ Drive_Keyboard_Sender/ … 送信側ESP32からキー入力でw/a/s/d操縦（要2台構成）
│  ├─ Car_Receiver/          … ★車体ESP32：ESP-NOW→サーボ4輪個別制御
│  ├─ Car_Servo_Diag/        … 車体サーボ単体診断（無線なし・切り分け用）
│  ├─ Car_Pin_Identify/      … 配線ピン特定テスト（無線なし・1本ずつ動かす）
│  ├─ Car_Auto_Drive/        … ★車体単独自走テスト（無線なし・前後左右5秒ずつ自動）
│  ├─ Signal_Test_Sender/    … 固定信号（60,60）を送り続ける送信機（電源切り分け用）
│  └─ Car_Signal_Test/       … 信号受信で後輪左のみ回転（電池単独動作の切り分け用）
│
├─【PC側ツール】
│  └─ Dashboard/             … リアルタイム可視化
│     ├─ bridge.py           … シリアル→JSON中継サーバー（pyserial）
│     └─ index.html          … ブラウザUI（距離/振動/腕/画面変化/検知ランプ）
│
└─【別アプリ（参考）】
   └─ TabilyApp/             … React Native アプリ（別プロジェクト）
```

★ = 本番デモで使うメインのスケッチ

### データの流れ（無線操縦の全体像）

```
[曲げセンサー]
   ↓ A3アナログ読み取り
[SPRESENSE: Bridge_SPRESENSE]
   ↓ UART "left,right\n"（D01 → GPIO16）
[送信ESP32: Bridge_ESP_Sender]
   ↓ ESP-NOW（DriveCmd構造体をブロードキャスト）
[車体ESP32: Car_Receiver]
   ↓ PWM（4ピン個別）
[サーボ4輪]
```

## キャリブレーション値（忘れないこと）

### 曲げセンサー（MB090-N-221-A02 / A3ピン）

| 項目 | 値 |
|---|---|
| フラット時ADC（0%） | 840 以上（起動時に自動取得） |
| 100%閾値ADC | 580 以下 |
| BEND_MAX_DIFF | 260 |

※ 起動時にまっすぐの状態でリセットすること。曲げた状態で起動すると基準がズレる。

### 連続回転サーボ

| 項目 | μs |
|---|---|
| 停止 | 1500 |
| MIN速度（5%） | 1510 |
| MAX速度（100%） | 2500 |

## ピン配置

### SPRESENSE（察知コア・ブリッジ）

| 部品 | ピン | 備考 |
|---|---|---|
| 曲げセンサー | A3 | IOREF(1.8V)→センサー→A3→10kΩ→GND |
| 超音波 Trig | D08 | |
| 超音波 Echo | D09 | 直結（電圧分圧なし） |
| 振動モーター | D05(PWM) | 1kΩ→2SC1815→モーター |
| IMU SDA | D14 | MPU-9250 I2C |
| IMU SCL | D15 | MPU-9250 I2C |
| UART TX（ESP32へ） | D01 | Bridge_SPRESENSE使用時 |

### 送信側ESP32（Bridge_ESP_Sender）

| 接続先 | ピン | 備考 |
|---|---|---|
| SPRESENSE D01(TX) | GPIO16(RX2) | UARTブリッジ受信 |
| GND | GND | SPRESENSEと共通必須 |

### 車体ESP32（Car_Receiver / Car_Auto_Drive）― 実機確認済み

| サーボ | ピン | グループ |
|---|---|---|
| 後輪左 | GPIO13 | 左 |
| 前輪左 | GPIO26 | 左 |
| 後輪右 | GPIO14 | 右 |
| 前輪右 | GPIO25 | 右 |
| 電源 | USBポート | **モバイルバッテリーから給電**（乾電池→VNはWi-Fi使用時に不安定のため不採用） |
| GND | GND | 電池−・サーボGNDと共通 |

※サーボのVCCは引き続き乾電池4本（6V）から直結。ESP32本体のみモバイルバッテリー。

**⚠ SPRESENSE拡張ボードのI/O電圧は必ず3.3Vに**（ジャンパ確認）。5V運用はMPU-9250の定格（3.6V）超過で、指先用IMU②破損の原因となった。

**旋回ロジック**（`left`/`right` は -100〜100）：

| 動作 | left | right |
|---|---|---|
| 前進 | +N | +N |
| 後退 | -N | -N |
| 右旋回 | 0 | +N（左輪停止・右輪のみ前進） |
| 左旋回 | +N | 0（右輪停止・左輪のみ前進） |

## スケッチ一覧

| フォルダ | 内容 | 状態 |
|---|---|---|
| HDR_Camera_Capture | カメラ撮影確認（VGA） | ✅ 動作確認済み |
| Camera_MotionDetect | カメラ動体検知（フレーム差分） | ✅ 動作確認済み |
| Bend_Sensor_Test | 曲げセンサー単体テスト | ✅ 動作確認済み |
| Servo_Test | サーボ単体テスト（スイープ） | ✅ 動作確認済み |
| Bend_Servo_Control | 曲げ→サーボ速度連動 | ✅ 動作確認済み |
| Ultrasonic_Test | 超音波距離計測（US-015） | ✅ 動作確認済み |
| Vibration_Test | 振動モーター制御テスト | ✅ 動作確認済み |
| Ultrasonic_Vibration | 超音波×振動統合（近いほど強く） | ✅ 動作確認済み |
| ESP32_Blink | ESP32動作確認（Lチカ） | ✅ 動作確認済み |
| ESPNow_Receiver | ESP-NOW受信テスト | ✅ 動作確認済み |
| ESPNow_Sender | ESP-NOW送信テスト | ✅ 動作確認済み |
| Car_Receiver | 車体ESP32：ESP-NOW→サーボ4輪個別制御（実配線ピン訂正済み） | ⬜ 書き込み済み・実走未確認 |
| Car_Servo_Diag | 車体サーボ単体診断（無線なし・切り分け用） | ✅ 役目終了（ピン特定はCar_Pin_Identifyで実施） |
| Car_Pin_Identify | 配線ピン特定テスト（無線なし・1本ずつ） | ✅ 実機検証完了（13/14/25/26を確認） |
| Car_Auto_Drive | 車体単独自走テスト（無線なし・前後左右5秒ずつ） | 🆕 未実行 |
| Drive_Test_Sender | 車体テスト送信（スピンターン方式・旧仕様） | ✅ 動作確認済み（現行の片側停止方式とは別ロジック） |
| Drive_Keyboard_Sender | 送信側ESP32からキー操縦（要2台構成） | 🆕 未使用（Car_Auto_Driveを優先採用） |
| MPU9250_Motion | 9軸IMU：加速度・ジャイロ | ✅ 動作確認済み |
| IMU_Mag_Test | IMU+地磁気テスト（地磁気は非搭載のためスキップ） | ✅ 動作確認済み |
| IMU_Single_Test | IMU単体テスト（アドレス自動判定・毎秒再スキャン） | ✅ 診断で使用 |
| IMU_Dual_Test | IMU2個同時読み（手首0x68＋指先0x69） | ⬜ 交換品到着後に使用 |
| MPU9250_Reference_Test | 参照コード移植版（切り分け用） | ✅ 役目終了 |
| I2C_Scanner | I2C全アドレススキャン | ✅ 診断で使用 |
| I2C_Deep_Diag | バス異常/デバイス無応答の切り分け | ✅ 診断で使用 |
| I2C_Bus_Recovery | 電位確認・パルス試験・バスロック解除 | ✅ 診断で使用（オシロ併用） |
| Signal_Test_Sender | 固定信号送信機（電源切り分け用） | ✅ 動作確認済み |
| Car_Signal_Test | 信号受信で後輪左のみ回転 | ✅ 動作確認済み（モバイルバッテリー給電） |
| Sense_Core | 察知コア統合（超音波・振動・カメラ・IMU） | ✅ 動作確認済み |
| Bridge_SPRESENSE | 曲げセンサー値をUARTで送信 | ✅ 疎通確認済み |
| Bridge_ESP_Sender | UART受信→ESP-NOW中継 | ✅ 疎通確認済み |
| Dashboard | Python中継サーバー＋ブラウザUI | ⬜ 実機未確認 |
| Audio_Test | ビープ音量テスト | ⬜ 未テスト |
| Nav_Compass_GPS | GPS・磁気テスト | ❌ 屋内/クローン品のため保留 |

## ビルド環境

- arduino-cli v1.5.1
- SPRESENSE:spresense v3.4.7 … ボード `SPRESENSE:spresense:spresense`
- esp32:esp32 v3.3.10（ラジコン用）… ボード `esp32:esp32:esp32`
- ポートは環境によって変わる（`arduino-cli board list` で確認）

## ESP-NOW メモ

- ESP32 #1（送信ブリッジ）MAC: `fc:f5:c4:1a:47:dc`
- ESP32 #2（車体）MAC: `24:0a:c4:f2:6d:3c`
- ブロードキャスト送信（`FF:FF:FF:FF:FF:FF`）で疎通確認済み

## ダッシュボードの起動方法

```bash
pip install pyserial
# Arduinoのシリアルモニタを閉じてから実行
python Dashboard/bridge.py
# ブラウザで http://localhost:8000 を開く
```

Sense_Core を SPRESENSE に書き込んだ状態で起動すること。
