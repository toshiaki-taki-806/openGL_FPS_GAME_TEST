#pragma once
#include "glm/glm.hpp"

// ==== カメラ情報構造体 ====
struct Camera {
	glm::vec3 pos;     // 位置
	glm::vec3 front;   // 前方向ベクトル
	glm::vec3 up;      // 上方向ベクトル
	float yaw;         // Y軸回転角（水平角）
	float pitch;       // X軸回転角（垂直角）
};

// ==== 外部変数 ====
extern Camera camera;

// ==== プロトタイプ関数 ====
void initCamera();
void updateCameraFront();
void applyCameraView();
