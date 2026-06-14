/**
 * HDR_Camera_Capture.ino
 *
 * SPRESENSE HDRカメラボード（CXD5602PWBCAM2W）で
 * 静止画1枚を撮影し、取得確認を行うサンプルスケッチ。
 *
 * 【対応ハードウェア】
 *   - SPRESENSEメインボード  : CXD5602PWBMAIN1
 *   - SPRESENSE拡張ボード    : CXD5602PWBEXT1
 *   - HDRカメラボード        : CXD5602PWBCAM2W（120dB）
 *
 * 【SDカード切り替え方法】
 *   SDカードが使えるときは下の行を有効（#define USE_SD）にする。
 *   SDカードなしの場合はコメントアウトするとシリアルログのみになる。
 *
 * 【チームへのメモ】
 *   今回はまず「カメラから画像を取得できる」ことの確認を最優先にしています。
 *   HDR撮影・フレーム差分・物体検知は次のステップで追加予定です。
 */

// ========================================================
// ★ SD使用 / 未使用の切り替えライン（1行で切り替え可能）
// ========================================================
#define USE_SD   // SDカードに保存する場合は有効、不要ならコメントアウト

// ========================================================
// ライブラリのインクルード
// ========================================================
#include <Camera.h>  // SPRESENSE カメラライブラリ

#ifdef USE_SD
  #include <SDHCI.h>   // SPRESENSE SD カードライブラリ
  SDClass SD;          // SDオブジェクトの宣言
#endif

// ========================================================
// 設定値（変更が必要な場合はここを編集する）
// ========================================================

// 撮影解像度（まず確実に動くVGAで設定）
// 変更候補: CAM_IMGSIZE_QVGA_H/V（320x240）、CAM_IMGSIZE_HD_H/V（1280x720）
const int IMG_WIDTH  = CAM_IMGSIZE_VGA_H;   // 640ピクセル
const int IMG_HEIGHT = CAM_IMGSIZE_VGA_V;   // 480ピクセル

// 画像フォーマット（JPEGが最も標準的で確認しやすい）
const CAM_IMAGE_PIX_FMT IMG_FORMAT = CAM_IMAGE_PIX_FMT_JPG;

// JPEG圧縮品質（75が標準。数値が大きいほど高画質・サイズ大）
const int JPEG_QUALITY = 75;

// SDカードへの保存ファイル名
const char* SAVE_FILENAME = "capture.jpg";

// シリアル通信速度
const int SERIAL_BAUD = 115200;

// ========================================================
// グローバル変数
// ========================================================
bool cameraReady = false;  // カメラ初期化成功フラグ
bool sdReady     = false;  // SD初期化成功フラグ（USE_SD時のみ使用）

// ========================================================
// コールバック関数（Camera.h が要求するエラーハンドラ）
// ※ 撮影中にエラーが起きたときに自動で呼ばれる
// ========================================================
void cameraErrorCallback(CamErr err) {
  Serial.print("[カメラ ERROR] コード: ");
  Serial.println((int)err);

  // エラーコードの意味を日本語で補足
  switch (err) {
    case CAM_ERR_NO_DEVICE:       Serial.println("  -> カメラが見つかりません。接続を確認してください。"); break;
    case CAM_ERR_ILLEGAL_DEVERR:  Serial.println("  -> カメラデバイスエラー"); break;
    case CAM_ERR_ALREADY_INITIALIZED: Serial.println("  -> 既に初期化済みです"); break;
    case CAM_ERR_NOT_INITIALIZED: Serial.println("  -> 未初期化です"); break;
    case CAM_ERR_NOT_STILL_INITIALIZED: Serial.println("  -> Still設定が未完了です"); break;
    case CAM_ERR_CANT_CREATE_THREAD: Serial.println("  -> スレッド生成失敗"); break;
    case CAM_ERR_INVALID_PARAM:   Serial.println("  -> パラメータが不正です"); break;
    case CAM_ERR_NO_MEMORY:       Serial.println("  -> メモリ不足"); break;
    case CAM_ERR_USR_INUSED:      Serial.println("  -> リソース使用中"); break;
    case CAM_ERR_NOT_PERMITTED:   Serial.println("  -> 操作が許可されていません"); break;
    default:                       Serial.println("  -> 不明なエラー"); break;
  }
}

// ========================================================
// setup() -- 起動時に1回だけ実行される
// ========================================================
void setup() {

  // ---- [1] シリアル通信の開始 ----
  Serial.begin(SERIAL_BAUD);

  // シリアルモニタが開くまで最大3秒待つ（PC接続時の取りこぼし防止）
  unsigned long startWait = millis();
  while (!Serial && (millis() - startWait < 3000)) {
    ;
  }

  Serial.println("========================================");
  Serial.println("  SPRESENSE HDRカメラ 撮影確認スケッチ");
  Serial.println("========================================");
  Serial.println();

  // ---- [2] SDカードの初期化（USE_SD が定義されている場合のみ） ----
#ifdef USE_SD
  Serial.print("[SD] 初期化中... ");
  if (SD.begin()) {
    Serial.println("OK");
    sdReady = true;
  } else {
    // SDが挿入されていない・認識できない場合は警告のみ出してスケッチを続行
    Serial.println("失敗 ※SDカードが見つかりません。シリアルログのみで動作します。");
    sdReady = false;
  }
#else
  Serial.println("[SD] USE_SD未定義 -> シリアルログモードで動作します");
#endif

  // ---- [3] カメラライブラリの初期化 ----
  Serial.print("[カメラ] 初期化中... ");

  CamErr err = theCamera.begin(
    1,                    // バッファ枚数（静止画1枚なので1で十分）
    CAM_VIDEO_FPS_NONE,   // ビデオストリームは使わない（静止画専用）
    IMG_WIDTH,            // 解像度 幅
    IMG_HEIGHT,           // 解像度 高さ
    CAM_IMAGE_PIX_FMT_YUV422,  // プレビュー用フォーマット（内部用、変更不要）
    JPEG_QUALITY          // JPEG品質
  );

  if (err != CAM_ERR_SUCCESS) {
    Serial.println("失敗！");
    Serial.print("  エラーコード: ");
    Serial.println((int)err);
    Serial.println("  -> カメラの接続・向き・コネクタを確認してください。");
    cameraReady = false;
    return;  // カメラが使えないので setup() を終了
  }

  Serial.println("OK");
  cameraReady = true;

  // ---- [4] 静止画フォーマットの設定 ----
  Serial.print("[カメラ] 静止画フォーマット設定中... ");

  err = theCamera.setStillPictureImageFormat(
    IMG_WIDTH,    // 幅
    IMG_HEIGHT,   // 高さ
    IMG_FORMAT    // JPEG形式
  );

  if (err != CAM_ERR_SUCCESS) {
    Serial.println("失敗！");
    Serial.print("  エラーコード: ");
    Serial.println((int)err);
    cameraReady = false;
    return;
  }

  Serial.println("OK");

  // ---- [5] エラーコールバックの登録 ----
  // 撮影中にエラーが起きた場合、cameraErrorCallback() が自動で呼ばれる
  theCamera.onError = cameraErrorCallback;

  // ---- [6] 初期化完了メッセージ ----
  Serial.println();
  Serial.println("[セットアップ完了]");
  Serial.print("  解像度: ");
  Serial.print(IMG_WIDTH);
  Serial.print(" x ");
  Serial.println(IMG_HEIGHT);
  Serial.print("  フォーマット: JPEG (品質=");
  Serial.print(JPEG_QUALITY);
  Serial.println(")");
  Serial.println();
  Serial.println("-> 3秒後に撮影を開始します...");
  delay(3000);  // カメラが安定するまで少し待つ
}

// ========================================================
// loop() -- setup() の後、繰り返し実行される
// ========================================================
void loop() {

  // カメラが正常に初期化できていない場合は何もしない
  if (!cameraReady) {
    Serial.println("[エラー] カメラが使用できません。リセットボタンを押してください。");
    delay(5000);
    return;
  }

  // ---- [7] 静止画の撮影 ----
  Serial.println("[撮影] シャッターを切ります...");

  CamImage photo = theCamera.takePicture();  // 撮影実行

  // ---- [8] 撮影結果の確認 ----
  if (!photo.isAvailable()) {
    // 撮影に失敗した場合
    Serial.println("[撮影] 失敗！ 画像データが取得できませんでした。");
    Serial.println("  考えられる原因:");
    Serial.println("  - カメラコネクタの接触不良");
    Serial.println("  - メモリ不足（バッファサイズを減らしてみてください）");
    delay(3000);
    return;
  }

  // 撮影成功 → 取得した画像情報をシリアルに表示
  Serial.println("[撮影] 成功！");
  Serial.print("  画像サイズ（バイト）: ");
  Serial.println(photo.getImgSize());
  Serial.print("  幅: ");
  Serial.print(photo.getWidth());
  Serial.print(" px / 高さ: ");
  Serial.print(photo.getHeight());
  Serial.println(" px");

  // ---- [9] 画像の保存 ----

#ifdef USE_SD
  if (sdReady) {
    // --- SD保存モード ---
    Serial.print("[SD] 保存中: ");
    Serial.print(SAVE_FILENAME);
    Serial.print(" ... ");

    // 同名ファイルが既にある場合は削除して上書き
    if (SD.exists(SAVE_FILENAME)) {
      SD.remove(SAVE_FILENAME);
    }

    // ファイルを開いてバイナリ書き込み
    File imgFile = SD.open(SAVE_FILENAME, FILE_WRITE);

    if (!imgFile) {
      Serial.println("失敗！ ファイルを開けませんでした。");
    } else {
      imgFile.write(photo.getImgBuff(), photo.getImgSize());
      imgFile.close();
      Serial.println("OK");
      Serial.print("  -> SDカードに保存しました: ");
      Serial.println(SAVE_FILENAME);
    }

  } else {
    // SDが初期化できていない場合はシリアルログのみ
    Serial.println("[SD] SDカードが使用不可のため保存はスキップしました。");
  }

#else
  // --- SDなしモード: シリアルログのみ ---
  Serial.println("[ログ] USE_SDが無効のため、SDへの保存はスキップしました。");
  Serial.println("  画像データを取得できていることはサイズで確認できます。");
#endif

  // ---- [10] 完了メッセージ ----
  Serial.println();
  Serial.println("============================================================");
  Serial.println("  撮影・確認が完了しました。スケッチを停止します。");
  Serial.println("  再撮影したい場合は SPRESENSE のリセットボタンを押してください。");
  Serial.println("============================================================");

  // 1回だけ撮影すればよいので、以降は無限ループ（何もしない）
  while (true) {
    delay(10000);
  }
}
