/**
 * Camera_MotionDetect.ino
 *
 * HDRカメラのストリーミング映像から「前方の変化／動き」を検知するスケッチ。
 * SenseReactの動作フローでカメラが担う「察知」の基礎。
 *
 * 【仕組み（精度版）】
 *   1. カメラを QVGA / YUV422 で映像ストリーミング（30fps＝HDRカメラの上限）
 *   2. コールバックで輝度(Y)を8pxごとに間引いてサンプリング（1200点）
 *   3. セルごとに直前フレームとの輝度差を計算
 *      - 平均輝度差（全体の明るさ変化の目安）
 *      - 変化セル数 → 画面の何%が変化したか（局所的な動きに強い）
 *   4. 変化領域%でON/OFF判定（ヒステリシスでチラつき防止）
 *
 * 【次のステップ】
 *   - 変化が収まったら（=静止したら）takePictureで撮影
 *   - セルを領域ごとに集計して「どこが動いたか」=物体検知へ発展
 */

#include <Camera.h>

const int SERIAL_BAUD = 115200;

// ---- ダウンサンプリング設定（QVGA=320x240前提）----
const int IMG_W   = 320;            // QVGA幅
const int IMG_H   = 240;            // QVGA高
const int STEP    = 8;              // 8pxごとにサンプリング（細かいほど高精度）
const int GRID_W  = IMG_W / STEP;   // 40
const int GRID_H  = IMG_H / STEP;   // 30
const int SAMPLES = GRID_W * GRID_H; // 1200サンプル

// ---- 検知パラメータ ----
const int CELL_THRESHOLD = 12;  // この輝度差を超えたセルを「変化した」とみなす
const int MOTION_ON_PCT  = 4;   // 変化領域がこの%以上で「動き検知」ON
const int MOTION_OFF_PCT = 2;   // この%未満まで下がったらOFF（ヒステリシス）

// プリント間引き（fpsに依存せず約100 msごと＝10Hz表示）
const unsigned long PRINT_INTERVAL_MS = 100;
unsigned long lastPrint = 0;

// 直前フレームの輝度サンプル
uint8_t prevY[SAMPLES];
bool prevReady = false;
bool motionState = false;  // ヒステリシス用の現在状態

// ========================================================
// 映像フレームごとに呼ばれるコールバック
// ========================================================
void CamCB(CamImage img) {
  if (!img.isAvailable()) return;

  uint8_t* buf = img.getImgBuff();  // YUV422: [Y0 U Y1 V ...]、Yは偶数バイト

  long sad = 0;          // 輝度差の総和
  int  changedCells = 0; // 大きく変化したセル数
  int  idx = 0;

  for (int gy = 0; gy < GRID_H; gy++) {
    int y = gy * STEP;
    for (int gx = 0; gx < GRID_W; gx++) {
      int x = gx * STEP;
      uint8_t Y = buf[(y * IMG_W + x) * 2];   // 輝度成分
      if (prevReady) {
        int d = abs((int)Y - (int)prevY[idx]);
        sad += d;
        if (d > CELL_THRESHOLD) changedCells++;
      }
      prevY[idx] = Y;
      idx++;
    }
  }
  prevReady = true;

  int avgDiff    = sad / SAMPLES;                 // 平均輝度差
  int changedPct = changedCells * 100 / SAMPLES;  // 変化領域（％）

  // ヒステリシス判定
  if (!motionState && changedPct >= MOTION_ON_PCT)  motionState = true;
  else if (motionState && changedPct <  MOTION_OFF_PCT) motionState = false;

  // 約150msごとに表示
  unsigned long now = millis();
  if (now - lastPrint >= PRINT_INTERVAL_MS) {
    lastPrint = now;

    int barLen = constrain(changedPct, 0, 20);  // 1マス=1%、最大20
    String bar = "";
    for (int i = 0; i < 20; i++) bar += (i < barLen) ? "#" : "-";

    Serial.print("変化領域: ");
    if (changedPct < 10) Serial.print(" ");
    Serial.print(changedPct);
    Serial.print("% [");
    Serial.print(bar);
    Serial.print("]  平均差:");
    Serial.print(avgDiff);
    Serial.print("  -> ");
    Serial.println(motionState ? "▲ 動き検知" : "静止");
  }
}

// ========================================================
// カメラ初期化（fpsを指定。失敗時はCamErrを返す）
// ========================================================
CamErr beginCamera(CAM_VIDEO_FPS fps) {
  return theCamera.begin(
    1,
    fps,
    CAM_IMGSIZE_QVGA_H,
    CAM_IMGSIZE_QVGA_V,
    CAM_IMAGE_PIX_FMT_YUV422
  );
}

void setup() {
  Serial.begin(SERIAL_BAUD);
  unsigned long t = millis();
  while (!Serial && (millis() - t < 3000)) { ; }

  Serial.println("========================================");
  Serial.println("  HDRカメラ フレーム差分 動体検知（精度版）");
  Serial.println("========================================");
  Serial.print("  サンプル数: ");
  Serial.print(SAMPLES);
  Serial.print(" (");
  Serial.print(GRID_W); Serial.print("x"); Serial.print(GRID_H);
  Serial.println(")");

  // ---- HDRカメラ(ISX019)は30fpsが上限。直接30fpsで初期化する ----
  // ※失敗したbegin()の後にend()→再begin()するとNO_DEVICEになるため、最初から30fps固定
  Serial.print("[カメラ] 初期化中（30fps）... ");
  CamErr err = beginCamera(CAM_VIDEO_FPS_30);
  if (err != CAM_ERR_SUCCESS) {
    Serial.println("失敗！");
    Serial.print("  エラーコード: ");
    Serial.println((int)err);
    return;
  }
  Serial.println("OK");

  Serial.print("[カメラ] ストリーミング開始... ");
  err = theCamera.startStreaming(true, CamCB);
  if (err != CAM_ERR_SUCCESS) {
    Serial.println("失敗！");
    Serial.print("  エラーコード: ");
    Serial.println((int)err);
    return;
  }
  Serial.println("OK");
  Serial.println("----------------------------------------");
  Serial.println("→ カメラの前で手を動かすと反応します");
}

void loop() {
  // 検知処理はコールバック側で実行されるため、ここは待つだけ
  delay(1000);
}
