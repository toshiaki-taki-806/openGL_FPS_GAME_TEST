#pragma once
#include <vector>
#include "glm/glm.hpp"
#include "camera.h"
#include "Material.h"
#include "wall.h"

// 武器デフォルト値
static float fireInterval = 0.2f;		//発射間隔のデフォルト値
const float airDensity = 1.225f;		// 空気:1.225f kg/m³,水:1000.0f kg/m³
const float dragCoefficient = 0.295f;	// 球体の形状
const glm::vec3 GUN_MUZZLE_OFFSET(-0.2f, -0.3f, 1.2f);
const float GUN_RADIUS = 0.02f;

// ---- 他ファイルから参照する外部変数・関数 ----
double getTimeSec();					// main.cppで定義された時間取得関数
extern const double PHYSICS_INTERVAL;	// main.cppで定義された物理演算の時間間隔
extern const float GROUND_Y;			// main.cppで定義された地面の座標
extern const float GRAVITY;				// main.cppで定義された重力加速度
extern const float STAND_HEIGHT;
extern const float CROUCH_HEIGHT;

// 弾の定義
struct Sphere {
	glm::vec3 position;				// 弾の位置
	glm::vec3 prevPos;				// 衝突判定に使用
	glm::vec3 originPos;			// 弾の発射位置
	glm::vec3 dir;					// 射撃方向（単位ベクトル）
	glm::vec3 velocity;				// 弾の速度ベクトル（初速 + 慣性）
	glm::vec3 inheritedVel;			// プレイヤーの速度を加算
	float length = 500.0f;			// 射程 m
	double spawnTime = 0.0;			// 発射した時刻 s
	double duration = 10.0;			// 生存時間 s
	float sphereRadius = 5.56f/1000/2;	// 弾の半径 m
	float renderScale = 100.0f;		// 描画倍率
	float speed = 120.0f;			// 初速 m/s 5.56*45mm NATO弾:940.0f
	float mass = 4.02f / 1000;		// 弾の重量gからkgへ 5.56*45mm NATO弾:4.02f g
	bool initialized = false;		// 初回初期化フラグ
};
extern std::vector<Sphere> spheres; // 連射に対応するため動的配列とする

struct DebugSphereInfo {
	glm::vec3 position;		// 消えた弾の座標
	float speed;			// 速度の大きさm/s
	double lifeTime;		// 生存時間 s
	double expireTime;		// 表示終了時刻
};
extern std::vector<DebugSphereInfo> debugSpheres;

//　レーザーポインタの定義
struct LaserPointer {
	bool active = false;
	glm::vec3 startPos;      // 発射位置
	glm::vec3 dir;           // 発射方向（正規化済み推奨）
	glm::vec3 offset = glm::vec3(-0.2f, -0.4f, 1.4f); // カメラ基準オフセット
	float length = 100.0f;     // 最大長さ
	float radius = 0.005f;    // 円柱半径
};
extern LaserPointer laserPointer;

// プロトタイプ関数宣言
// 発射関数
void updateSpheres();
bool SweepSphereAABB(const Sphere& s, const Wall& w);
void drawSpheres();
void fireSphere();
void drawLaserPointer();
void drawGunMuzzle(float radius);
void resolveGunLineCollision(const std::vector<Wall>& walls);
void resolvePlayerCollision(const std::vector<Wall>& walls);
float getFireInterval();		// 連射間隔を取得
void setFireInterval(float f);	// 設定も可能
void handleCrouchAndCeiling(const std::vector<Wall>& walls);