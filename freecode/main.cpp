#include "glut.h"
#include "glm/glm.hpp"
#include <cmath>
#include <ctime>
#include "weapon.h"
#include "ground.h"
#include "wall.h"
#include "camera.h"

// ==== 個人設定関連 ====
int crosshairSize = 5;             // クロスヘアのサイズ（画素単位、半分の長さ）
unsigned char crosshairColor[3] = { 0x00, 0xff, 0x00 }; // クロスヘアの色（RGB）
float crosshairThickness = 1.5f;   // クロスヘアの線の太さ
float CAMERA_FOVY = 60.0f;         // 垂直方向の視野角（度）
float CAMERA_ZFAR = 500.0f;        // カメラの描画可能な最遠距離

const float MOUSE_SENSITIVITY = 0.05f;	//　マウスの感度
const float MOVE_SPEED = 4.0f;	// 通常の移動速度　m/s
const float DASH_SPEED = 6.0f;	// ダッシュ時の速度 m/s
float speed_mps;				// 速度をメートル　毎　秒とする計算用変数
bool isDashing = false;			// ダッシュフラグ
const float P_RADIUS = 0.5f;	// 身体の半径

//　しゃがみ関連
bool isCrouching = false;		//　しゃがみフラグ
const float STAND_HEIGHT = 1.53f;	// 目線の高さ
const float CROUCH_HEIGHT = 0.95f;	// しゃがみ時の目線の高さ
const float CROUCH_SPEED = 1.5f;	// しゃがみ時の速度　m/s

// ==== 重力関連 ====
float velocityY = 0.0f;				// 重力による速度計算用変数
bool onGround = true;				// 地面への接地フラグ
const float GRAVITY = 9.81f;		// 重力加速度
const float JUMP_POWER = 3.1f;		// ジャンプの初速度　垂直飛び0.5m相当
const float GROUND_Y = 0.0f;		// 地面の高さ	

// ==== ウィンドウ関連 ====
int windowWidth = 1280;				// 画面の横幅
int windowHeight = 720;				//画面高さ
int windowCenterX, windowCenterY;	// 画面中央を示す変数
bool ignoreNextWarp = false;		// マウス移動の更新無視フラグ

// ==== 時間管理 ====
double lastFrameTime = 0.0;			// 前回描写時の時間を記録
double lastPhysicsTime = 0.0;		// 前回物理演算時の時間を記録
const double FRAME_FPS = 60.0;		// 描写速度　FPS
const double PHYSICS_FPS = 240.0;	// 物理演算の速度 FPS
const double FRAME_INTERVAL = 1.0 / FRAME_FPS;		// 次の描写までの間隔
const double PHYSICS_INTERVAL = 1.0 / PHYSICS_FPS;	// 次の物理演算までの間隔

// キー状態管理
bool keyState[256] = { false };		// キーボードが押されたままかの状態
// マウス　左=0, 中=1, 右=2
bool mouseState[3] = { false };		// マウスのボタンの状態

static int mouseDx;
static int mouseDy;

// 時間取得関数=glutInit()からの経過時間を秒単位で返す関数
double getTimeSec() {
	return glutGet(GLUT_ELAPSED_TIME) / 1000.0;
}

// ラジアン変換 角度℃をラジアンへ変換　インライン関数にして処理を軽減
inline float toRad(float deg) { return glm::radians(deg); }

//　慣性関連
glm::vec3 playerVelocity(0.0f);		// プレイヤーの速度。慣性の計算で利用

// ==== クロスヘア ====
// 画面中央に十字のマークを描画する関数
void drawCrosshair()
{
	glMatrixMode(GL_PROJECTION);        // 射影行列モードに切り替え
	glPushMatrix();                      // 現在の射影行列を退避
	glLoadIdentity();                    // 射影行列を初期化
	gluOrtho2D(0, windowWidth, 0, windowHeight); // 2Dスクリーン座標系に設定

	glMatrixMode(GL_MODELVIEW);          // モデルビュー行列モードに切り替え
	glPushMatrix();                      // 現在のモデルビュー行列を退避
	glLoadIdentity();                    // モデルビュー行列を初期化

	glDisable(GL_DEPTH_TEST);            // 深度テストを無効化（必ず手前に描画）

	glColor3ub(crosshairColor[0], crosshairColor[1], crosshairColor[2]);		// 個人設定の色
	glLineWidth(crosshairThickness); // 個人設定の太さ

	int cx = windowWidth / 2;            // 画面中央のX座標
	int cy = windowHeight / 2;           // 画面中央のY座標

	glBegin(GL_LINES);                    // 線を描画開始
	glVertex2i(cx - crosshairSize, cy);       // 左方向の端
	glVertex2i(cx + crosshairSize, cy);       // 右方向の端
	glVertex2i(cx, cy - crosshairSize);       // 下方向の端
	glVertex2i(cx, cy + crosshairSize);       // 上方向の端
	glEnd();                              // 線の描画終了

	glEnable(GL_DEPTH_TEST);             // 深度テストを再有効化

	glPopMatrix();                       // モデルビュー行列を復元
	glMatrixMode(GL_PROJECTION);         // 射影行列モードに切り替え
	glPopMatrix();                       // 射影行列を復元
	glMatrixMode(GL_MODELVIEW);          // 元のモデルビュー行列モードに戻す

}

// 画面右上に任意の文字列を描画する関数
void drawText2D(const char* text, int x, int y, void* font = GLUT_BITMAP_HELVETICA_18, unsigned char r = 255, unsigned char g = 255, unsigned char b = 255)
{
	// --- 2D描画用に射影行列切り替え ---
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, windowWidth, 0, windowHeight);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST); // 手前に表示
	glColor3ub(r, g, b);

	glRasterPos2i(x, y);
	for (int i = 0; text[i] != '\0'; ++i)
		glutBitmapCharacter(font, text[i]);

	glEnable(GL_DEPTH_TEST);

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
}

void drawCameraInfo()
{
	char buffer[128];

	// 座標
	_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, "Pos: X: %.2f Y: %.2f Z: %.2f", camera.pos.x, camera.pos.y, camera.pos.z);
	drawText2D(buffer, 20, windowHeight - 20);

	// 前方向ベクトル
	_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, "Dir: X: %.2f Y: %.2f Z: %.2f", camera.front.x, camera.front.y, camera.front.z);
	drawText2D(buffer, 20, windowHeight - 40);

	// Yaw/Pitch角度
	_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, "Yaw: %.2f Pitch: %.2f", camera.yaw, camera.pitch);
	drawText2D(buffer, 20, windowHeight - 60);

	// 速度
	_snprintf_s(buffer, sizeof(buffer), _TRUNCATE, "Speed: %f", speed_mps);
	drawText2D(buffer, 20, windowHeight - 80);
}

void drawDebugSpheres()
{
	char buffer[128];
	double currentTime = getTimeSec();

	// 右上に縦に表示
	int lineHeight = 20;
	int startY = windowHeight - 20;

	for (auto it = debugSpheres.begin(); it != debugSpheres.end(); ) {
		// 表示期限が過ぎたら消す
		if (currentTime > it->expireTime) {
			it = debugSpheres.erase(it);
			continue;
		}

		_snprintf_s(buffer, sizeof(buffer), _TRUNCATE,
			"POS : X=%.2f Y=%.2f Z=%.2f | Life:%.2f s |SPEED:%.2f m/s",
			it->position.x, it->position.y, it->position.z, it->lifeTime, it->speed);

		drawText2D(buffer, windowWidth - 600, startY); // 右上表示
		startY -= lineHeight;

		++it;
	}
}

// 地形の設定
void setupScene()
{
	// 壁を追加（位置x,y,z 幅 高さ 奥行 色RGB）
	addWall(glm::vec3(0.0f, 1.5f, 10.0f), 2.0f, 3.0f, 0.2f, glm::vec3(0.0f, 0.0f, 0.0f), MaterialType::Concrete);
	addWall(glm::vec3(3.0f, 2.0f, 15.0f), 1.0f, 4.0f, 0.3f, glm::vec3(0.0f, 0.0f, 0.0f), MaterialType::RigidBody);
}


// ==== 描画関数 ====
void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	// GLMのlookAtを使ってビュー行列を設定
	applyCameraView();

	drawGround();					// 地面
	drawWalls();					// 壁
	//drawCrosshair();				// クロスヘア
	drawCameraInfo();				// デバッグ情報
	drawGunMuzzle(GUN_RADIUS);		// 銃
	drawSpheres();					// 弾丸
	drawDebugSpheres();				// 弾のデバッグ情報
	drawLaserPointer();				// 常時照射レーザーポインタ
	glutSwapBuffers();
}


// ウィンドウサイズ変更時に呼ばれるコールバック関数
void reshape(int w, int h)
{
	if (h == 0) h = 1;                  // 高さが0になると割り算エラーになるため補正
	float ratio = w * 1.0f / h;         // アスペクト比（幅÷高さ）

	// --- 投影行列の設定 ---
	glMatrixMode(GL_PROJECTION);        // 投影行列モードに切り替え
	glLoadIdentity();                    // 投影行列をリセット
	gluPerspective(CAMERA_FOVY, ratio, 0.1, CAMERA_ZFAR); // FOV, アスペクト比, 近距離, 遠距離
	glMatrixMode(GL_MODELVIEW);         // モデルビュー行列モードに戻す

	// ウィンドウ情報の更新
	windowWidth = w;
	windowHeight = h;
	windowCenterX = w / 2;              // ウィンドウ中心X座標
	windowCenterY = h / 2;              // ウィンドウ中心Y座標
	glutWarpPointer(windowCenterX, windowCenterY); // マウスを中央に移動
}

// ==== マウス入力 ====
// マウスの移動で視点（カメラの向き）を更新する
void processMouseMotion(int x, int y)
{
	// WarpPointerで中央に戻したイベントを無視
	if (ignoreNextWarp)
	{
		ignoreNextWarp = false; // 次回は通常処理に戻す
		return;
	}

	// マウスの移動量を計算
	mouseDx += x - windowCenterX; // X方向の差分
	mouseDy += y - windowCenterY; // Y方向の差分

	// マウスを再びウィンドウ中央に戻す
	glutWarpPointer(windowCenterX, windowCenterY);
	ignoreNextWarp = true; // このWarpによるイベントを無視
	//glutPostRedisplay();   // 描画更新要求
}

// マウスのクリック　左=0, 中=1, 右=2
void mouseClick(int button, int state, int x, int y) {
	int index = -1;
	switch (button) {
	case GLUT_LEFT_BUTTON:   index = 0; break;
	case GLUT_MIDDLE_BUTTON: index = 1; break;
	case GLUT_RIGHT_BUTTON:
		index = 2;
		if (state == GLUT_DOWN) {
			fireSphere();	// 弾丸発射
		}
		break;
	}

	// マウスが押されているかいないかを切り替える
	if (index != -1) {
		mouseState[index] = (state == GLUT_DOWN);
	}
}

// キー押下時
void keyDown(unsigned char key, int, int) {
	keyState[key] = true;

	if (key == ';') isDashing = true;
	if (key == ':') isCrouching = true;
	if (key == '@') laserPointer.active = !laserPointer.active; // 押すたび切替
}

// キー離した時
void keyUp(unsigned char key, int, int) {
	keyState[key] = false;

	if (key == ';') isDashing = false;
	if (key == ':') isCrouching = false;
}

// update関数内で移動処理
void handleMovement() {
	// crossで視点の右方向ベクトルを計算
	glm::vec3 right = glm::normalize(glm::cross(camera.front, camera.up));
	// 水平面のみの前方向ベクトル
	glm::vec3 flatFront = camera.front;
	flatFront.y = 0.0f;		//　上方向の影響を無くす
	flatFront = glm::normalize(flatFront);		//　正規化

	glm::vec3 moveDir(0.0f);		//　移動方向を(0,0,0)で初期化

	// キーの状態を取得することにより同時押しに対応
	if (keyState[27])  exit(0);					// ESCで終了
	if (keyState['i']) moveDir += flatFront;	// 前進
	if (keyState['k']) moveDir -= flatFront;	// 後退
	if (keyState['j']) moveDir -= right;		// 左
	if (keyState['l']) moveDir += right;		// 右
	
	// 斜め移動も自然になるよう正規化
	if (glm::length(moveDir) > 0.0f) {
		if (isCrouching)
			speed_mps = CROUCH_SPEED;
		else if (isDashing)
			speed_mps = DASH_SPEED;
		else
			speed_mps = MOVE_SPEED;

		// 毎フレームのプレイヤー速度を記録（地面にいるときのみ）
		camera.moveDir = normalize(moveDir);

		// normalize(moveDir)で方向のみで大きさ1へ変換。　m/sの速度を掛けて実際の速度を算出
		moveDir = normalize(moveDir) * speed_mps;
	}

	// 実際に移動
	camera.pos += moveDir * static_cast<float>(PHYSICS_INTERVAL);

	// ジャンプ
	if (keyState[' '] && onGround && !isCrouching) {
		velocityY = JUMP_POWER;
		onGround = false;
	}
	// 最終的なYを調整
	camera.pos.y = (onGround) ? GROUND_Y + (isCrouching ? CROUCH_HEIGHT : STAND_HEIGHT) : camera.pos.y;
}


// ==== 物理更新・描画制御 ====
// 描画と物理演算を別タイミングで更新する関数
void update()
{
	double current = getTimeSec(); // 現在時刻を秒単位で取得

	// --- 物理演算（一定間隔で複数回実行） ---
	while (current - lastPhysicsTime >= PHYSICS_INTERVAL)
	{	
		camera.prevPos = camera.pos;

		// カメラの角度を更新
		float dx = mouseDx;
		float dy = mouseDy;
		mouseDx = 0;
		mouseDy = 0;
		camera.prevYaw = camera.yaw;
		camera.prevPitch = camera.pitch;
		camera.yaw += dx * MOUSE_SENSITIVITY;   // 左右回転
		camera.pitch -= dy * MOUSE_SENSITIVITY; // 上下回転
		// ピッチ角の制限（真上/真下に向かないように）
		if (camera.pitch > 89.0f) camera.pitch = 89.0f;
		if (camera.pitch < -89.0f) camera.pitch = -89.0f;
		// GLMの前方向ベクトルを更新
		updateCameraFront();

		// --- キーによる移動（水平移動） ---
		handleMovement();

		// 壁との衝突判定
		resolveGunLineCollision(g_walls);
		resolvePlayerCollision(g_walls);

		// 空中にいる場合の重力処理
		if (!onGround)
		{
			velocityY -= GRAVITY * PHYSICS_INTERVAL;		// 重力加速度を垂直速度に加算
			camera.pos.y += velocityY * PHYSICS_INTERVAL;	// カメラのY座標を更新

			// 地面に接地した場合のリセット
			if (camera.pos.y <= GROUND_Y + STAND_HEIGHT)
			{
				camera.pos.y = GROUND_Y + STAND_HEIGHT;		// 地面+目線高さで止める
				velocityY = 0.0f;							// 垂直速度をリセット
				onGround = true;							// 接地フラグを立てる
			}
		}

		// 弾の演算
		updateSpheres();

		// --- 右クリック連射処理 ---
		static double lastShotTime = 0.0;
		if (mouseState[2])		// 右クリック押下中
		{
			double shotTime = getTimeSec();
			if (shotTime - lastShotTime >= getFireInterval())
			{
				fireSphere();
				lastShotTime = shotTime;
			}
		}
		else {
			lastShotTime = getTimeSec();
		}

		// 次の物理更新時間に進める
		lastPhysicsTime += PHYSICS_INTERVAL;
	}

	// --- 描画更新（60FPS間隔で再描画） ---
	if (current - lastFrameTime >= FRAME_INTERVAL)
	{
		glutPostRedisplay();       // display() の描画要求
		lastFrameTime = current;   // 最後に描画した時間を更新
	}
}


// ==== 初期化 ====
// OpenGL の描画設定を行う関数
void init()
{
	// 深度テストを有効にする
	// これにより奥行きに応じて物体が正しく重なって描画される
	glEnable(GL_DEPTH_TEST);

	// 背景色を設定（ここでは黒）
	// glClear() で画面をクリアするときにこの色で塗りつぶされる
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}


// ==== メイン ====
int main(int argc, char** argv)
{
	// GLUT の初期化（コマンドライン引数を渡す）
	glutInit(&argc, argv);

	// 表示モード設定
	// GLUT_DOUBLE: ダブルバッファリング（ちらつき防止）
	// GLUT_RGB: RGBカラーモード
	// GLUT_DEPTH: 深度バッファ有効（3D描画用）
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	// ウィンドウサイズの初期設定（幅×高さ）
	glutInitWindowSize(windowWidth, windowHeight);

	// 描画ウィンドウの位置を指定
	glutInitWindowPosition(600, 100);  

	// ウィンドウ作成とタイトル設定
	glutCreateWindow("FPS_TEST_GAME");

	// OpenGL初期化関数呼び出し（深度テストや背景色の設定など）
	init();

	// カメラの初期化関数呼び出し
	initCamera();

	// --- シーン初期化 ---
	setupScene();

	// 時間管理変数を初期化
	// 描画と物理更新の基準時刻として使用
	lastFrameTime = getTimeSec();
	lastPhysicsTime = getTimeSec();

	// 描画コールバック登録
	// 画面を描画する関数 display() を GLUT に登録
	glutDisplayFunc(display);

	// アイドルコールバック登録
	// CPUが空いたときに update() が呼ばれ、物理計算や描画要求を処理
	glutIdleFunc(update);

	// ウィンドウサイズ変更時のコールバック登録
	// 画面リサイズに応じて投影行列を再計算
	glutReshapeFunc(reshape);

	// キーボード入力コールバック登録
	glutKeyboardFunc(keyDown);
	glutKeyboardUpFunc(keyUp);

	// マウス移動コールバック登録（ボタンを押さなくても反応）
	// マウスで視点操作する処理を呼び出す
	glutPassiveMotionFunc(processMouseMotion); // ボタンなしでも呼ばれる
	glutMotionFunc(processMouseMotion);        // ボタン押下中でも呼ばれる

	// マウスクリックコールバック登録
	// マウスでクリックした処理を呼び出す
	glutMouseFunc(mouseClick);

	// マウスをウィンドウ中央に移動（視線移動の基準点）
	windowCenterX = windowWidth / 2;
	windowCenterY = windowHeight / 2;
	glutWarpPointer(windowCenterX, windowCenterY);

	// マウスカーソルを非表示にする（FPS視点用）
	glutSetCursor(GLUT_CURSOR_NONE);

	// GLUTのメインループ開始
	// ここから無限ループになり、登録したコールバックが呼ばれ続ける
	glutMainLoop();

	// 正常終了
	return 0;
}
