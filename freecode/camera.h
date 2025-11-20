#pragma once
#include "glm/glm.hpp"

// ==== カメラ情報構造体 ====
struct Camera {
	glm::vec3 pos;		// 位置
	glm::vec3 front;	// 前方向ベクトル
	glm::vec3 up;		// 上方向ベクトル
	glm::vec3 prevPos;	// 衝突判定に使用
	glm::vec3 moveDir;	// 移動方向
	float yaw;			// Y軸回転角（水平角）
	float pitch;		// X軸回転角（垂直角）
	float prevYaw;
	float prevPitch;
};

// ==== 外部変数 ====
extern Camera camera;

struct Player {
	glm::vec3 footPos;		// 足元の位置（物理・衝突判定用）
	bool onGround;			// 接地フラグ
	bool isCrouching;		// しゃがみ状態
	bool isDashing;			// ダッシュ状態
	float eyeHeight;		// 目線高さ（立ち/しゃがみで変化）
	float radius;			// カプセル半径
};
extern Player player;

// ==== プロトタイプ関数 ====
void initCamera();
void updateCameraFront();
void applyCameraView();
