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

## 進捗（2026-07-09）

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
- **指先センサーを「ジャイロのみ」方式に転換**：加速度不要、回転速度（dps）をそのまま旋回量に変換する設計へ。L3GD20（3.3V・ST社製3軸ジャイロ）を採用し `L3GD20_Gyro_Test` で動作確認
- **車体制御チェーンをESP32単体に統合**：曲げセンサー（ADC1・GPIO34）＋L3GD20（I2C・GPIO21/22）＋ESP-NOW送信を`Bend_Gyro_ESP_Sender`1本にまとめ、SPRESENSE↔ESP32のUARTブリッジは車体制御において不要に（SPRESENSEはSense_Core用途で引き続き独立稼働）
- 曲げセンサーの校正値をESP32の3.3V回路で実測し直し、双方向（前進+/後退-）のthrottleに対応
- **サーボ機種を特定・定格確認**：GeekServo 9G 360° Motor、動作電圧3.3〜6V（現状の乾電池4本＝6Vが上限と確定）
- 速度検証用に`Car_Max_Speed_Test`（無線なし・常時最大速度）を作成
- ESP32のVN/5Vピンの誤用（VNへの電源直結）による損傷リスクを未然に回避

**⬜ 残タスク（優先度順）**

| 優先 | タスク | 状態 |
|---|---|---|
| 🔴 必須 | **拡張ボードI/O電圧ジャンパを5V→3.3Vへ**（IMU破損の再発防止。SPRESENSE側でI2Cセンサーを繋ぐ前に必ず） | 未対応 |
| 🔴 高 | MVP統合デモ動作確認（`Bend_Gyro_ESP_Sender`単体で曲げ→ジャイロ→無線→4輪が実際に動く） | 未完 |
| 🟡 中 | 曲げセンサーの`BEND_POS/NEG_MAX_DIFF`・ジャイロ`MAX_DPS`の実地での最終調整 | 調整中 |
| 🟡 中 | サーボ速度アップ手段の検証（新品電池・配線抵抗低減・機械摩擦低減。電圧upは6V上限のため不可) | 検証中 |
| 🟡 中 | `Car_Auto_Drive` 実機検証（4輪が訂正後のピンで正しく動くか） | 未検証 |
| 🟡 中 | ダッシュボード実機動作確認（bridge.py + Sense_Core 同時起動） | 未確認 |
| 🟡 中 | 音声テスト（Audio_Test をイヤホン繋いで確認） | 未確認 |
| 🟢 低 | AR/VR拡張（中継用ESP32→PCサーバー→スマホWebAR）※構想段階 | 未着手 |
| 🟢 低 | 発表用デモ調整・見せ方整理 | 未着手 |
| ❌ 故障 | 指先用IMU②（MPU-9250）＝SDAパッド破損（5V I/O定格超過が原因）。**L3GD20ジャイロに方針転換したため交換調達は不要に** | 対応済み（方針転換） |
| ❌ 断念 | 地磁気センサー（クローン品のためAK8963非搭載・実装不可） | 断念 |
| ❌ 保留 | GPS方位（屋内でFix不可） | 保留 |

## 拡張予定：AR/VRでの操作（構想・未実装）

> `docs/EXPANSION_IDEAS.md`「Tier4 操作AR」を土台に、VRまで含めて具体化したもの。**構想段階であり実装は未着手。** MVP（曲げ→無線→物理カー）完成後、余力があれば着手する。

### コンセプト

曲げセンサー＋L3GD20ジャイロから生成している`left/right`の駆動信号は、**ハード非依存**の単なる数値ペアである。現在はESP-NOWで物理RCカーのサーボを動かしているが、**同じジェスチャー入力（かざす・指す・回す）で、AR/VR空間上の仮想オブジェクト（仮想ラジコン・ドローン等）を操作する**ことも技術的に可能。

- 物理カーが本番で不調の場合の**代替デモ経路**になる
- 装着者の「意図」を観客がAR越しに見られる＝察知だけでなく操作もデモ映えする
- 既存のダッシュボード資産（Python中継サーバー＋ブラウザUI）をほぼそのまま転用できる

### 技術構成（案）

```
[曲げセンサー + L3GD20] → ESP32(送信) --ESP-NOW--> ESP32(受信・PCにUSB接続)
                                                          ↓ シリアル(left,right)
                                                 Python中継サーバー（bridge.py拡張）
                                                          ↓ WebSocket/HTTP
                                                    ブラウザ（WebXR / AR.js）
                                                          ↓
                                            AR：スマホカメラ映像に3D仮想カーを重畳
                                            VR：フルCG空間内で仮想カーを操作
```

### 段階（Tier）

| Tier | 内容 | 必要な新規要素 |
|---|---|---|
| **A（AR・最有力）** | ブラウザでスマホのカメラ映像に仮想カー/ドローンを重畳し、`left/right`でその場で動かす | Three.js＋AR.js（またはWebXR）、受信用ESP32→PC中継 |
| **B（VR）** | 同じ信号でVRヘッドセット内の仮想カーを操作。フルCG空間のためマーカー不要、Tier Aよりむしろ単純 | WebXR（VRセッション）、簡易3Dシーン |
| **C（AI連携）** | Gemini物体認識（既存構想）と組み合わせ、認識した対象にARで仮想カーをロックオン走行させる | Gemini構想との統合 |

### 流用できる既存資産

| 資産 | 転用方法 |
|---|---|
| `DriveCmd{left, right}`構造体 | 変更不要でそのまま使える |
| ESP-NOW送信ロジック | `Bend_Gyro_ESP_Sender`をそのまま流用 |
| Python中継サーバー | `Dashboard/bridge.py`と同じ構成（シリアル受信→JSON配信）でOK |
| ブラウザ側ポーリング | `Dashboard/index.html`の150ms間隔ポーリングと同じ仕組みを転用可 |

推奨着手順は **Tier A → Tier B → Tier C**。Tier Aが実質「仮想カー版のCar_Receiver」であり、そこにVRセッション対応を足すだけでTier Bに拡張できるため、追加コストは小さい。

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
│  ├─ Bridge_SPRESENSE/      … 曲げ値をUARTでESP32へ送信（旧方式・現在は車体制御チェーンで不使用）
│  ├─ Audio_Test/            … ビープ音量テスト
│  └─ Nav_Compass_GPS/       … GPS・磁気テスト（保留）
│
├─【ESP32 無線系】
│  ├─ ESP32_Blink/           … ESP32動作確認（Lチカ）
│  ├─ ESPNow_Sender/         … ESP-NOW送信テスト（疎通確認用）
│  ├─ ESPNow_Receiver/       … ESP-NOW受信テスト（疎通確認用）
│  ├─ Bridge_ESP_Sender/     … UART受信→ESP-NOW中継（旧方式・現在は車体制御チェーンで不使用）
│  ├─ L3GD20_Gyro_Test/      … L3GD20ジャイロ単体テスト（アドレス自動判定・ゼロ点補正）
│  ├─ Bend_Gyro_ESP_Sender/  … ★送信側ESP32単体完結：曲げ(ADC)+ジャイロ(I2C)+ESP-NOW送信
│  ├─ Drive_Test_Sender/     … 車体テスト送信（スピンターン方式・旧仕様）
│  ├─ Drive_Keyboard_Sender/ … 送信側ESP32からキー入力でw/a/s/d操縦（要2台構成）
│  ├─ Car_Receiver/          … ★車体ESP32：ESP-NOW→サーボ4輪個別制御
│  ├─ Car_Servo_Diag/        … 車体サーボ単体診断（無線なし・切り分け用）
│  ├─ Car_Pin_Identify/      … 配線ピン特定テスト（無線なし・1本ずつ動かす）
│  ├─ Car_Auto_Drive/        … ★車体単独自走テスト（無線なし・前後左右5秒ずつ自動）
│  ├─ Car_Max_Speed_Test/    … 車体単独・常時最大速度テスト（電圧変更による速度検証用）
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

### データの流れ（無線操縦の全体像・現行）

```
[曲げセンサー]──GPIO34(ADC1)──┐
                              │
[L3GD20ジャイロ]──GPIO21/22(I2C)──┤
                              ↓
                  [送信ESP32: Bend_Gyro_ESP_Sender]
                              ↓ ESP-NOW（DriveCmd構造体をブロードキャスト）
                  [車体ESP32: Car_Receiver]
                              ↓ PWM（4ピン個別）
                        [サーボ4輪]
```

送信側はESP32単体で完結しており、SPRESENSEを経由しない（SPRESENSEはSense_Core用途で別稼働）。
`Bridge_SPRESENSE` / `Bridge_ESP_Sender`（旧UART中継方式）は現在未使用。

## キャリブレーション値（忘れないこと）

### 曲げセンサー（MB090-N-221-A02 / ESP32 GPIO34・3.3V回路）

`Bridge_SPRESENSE`（SPRESENSE・IOREF 1.8V基準）時代の値は現在未使用。**ESP32・3.3V回路での実測値**が以下。

| ADC値 | throttle |
|---|---|
| 1900 | +100% |
| 1400 | 0%（まっすぐ・起動時に自動取得） |
| 400 | -100% |

※ 起動時にまっすぐの状態でリセットすること。プラス方向とマイナス方向で曲げ幅が異なるため`BEND_POS_MAX_DIFF`(500)・`BEND_NEG_MAX_DIFF`(1000)を別々に設定。

### L3GD20ジャイロ（ステア）

| 項目 | 値 |
|---|---|
| フルスケール | ±245dps |
| ゼロ点補正 | 電源投入後の最初の1回のみ実施（再接続のたびには行わない） |
| MAX_DPS（ステア100%になる回転速度） | 50（感度2倍設定） |

### 連続回転サーボ（GeekServo 9G 360° Motor）

| 項目 | 値 |
|---|---|
| 動作電圧範囲 | **3.3V〜6V**（データシート確認済み・6Vが絶対上限） |
| 定格電圧 | 4.8V |
| 停止 | 1500μs |
| MAX速度（100%） | 2600μs（拡張レンジ・元は2500） |
| MIN（-100%） | 400μs（拡張レンジ・元は500） |

⚠ **乾電池4本＝6Vが電圧上限。5本(7.5V)以上は定格超過につき非推奨。**

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

### 送信側ESP32（Bend_Gyro_ESP_Sender）― 現行

| 部品 | ピン | 備考 |
|---|---|---|
| 曲げセンサー分圧点 | **GPIO34**（ADC1） | 3.3V→センサー→GPIO34→10kΩ→GND。Wi-Fi中はADC2系統が不安定なため必ずADC1を使用 |
| L3GD20 SDA | GPIO21 | Wireライブラリのデフォルト |
| L3GD20 SCL | GPIO22 | 同上 |
| L3GD20 CS | 3.3V | I2Cモード固定（HIGH固定必須） |
| L3GD20 SA0 | GND or 3.3V | アドレス0x6A/0x6B、コード側で自動判定 |
| ESP32本体の電源 | **5V(=VIN)ピン** | 電池+をここへ。**「VN」ピン（GPIO39・ADC専用）には絶対に繋がないこと** |

※旧`Bridge_ESP_Sender`（GPIO16でSPRESENSEからUART受信）は車体制御チェーンでは現在不使用。

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
※単体速度検証（`Car_Max_Speed_Test`、Wi-Fi不使用）の場合は電池のみでも安定動作可。その際もESP32本体の電源は**「5V」ピン**へ（「VN」はADC専用のため厳禁）。

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
| Car_Max_Speed_Test | 車体単独・常時最大速度（電圧変更の速度検証用） | ✅ 動作確認済み |
| L3GD20_Gyro_Test | L3GD20ジャイロ単体テスト | ✅ 動作確認済み |
| Bend_Gyro_ESP_Sender | ★曲げ+ジャイロ+ESP-NOW送信をESP32単体で完結 | ✅ 動作確認済み・調整中 |
| Sense_Core | 察知コア統合（超音波・振動・カメラ・IMU） | ✅ 動作確認済み |
| Bridge_SPRESENSE | 曲げセンサー値をUARTで送信 | ⬜ 現在不使用（Bend_Gyro_ESP_Senderに統合） |
| Bridge_ESP_Sender | UART受信→ESP-NOW中継 | ⬜ 現在不使用（Bend_Gyro_ESP_Senderに統合） |
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
