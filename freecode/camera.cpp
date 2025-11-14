#include "glut.h"
#include "glm/gtc/matrix_transform.hpp"
#include "camera.h"

// ==== グローバルカメラインスタンス ====
Camera camera;

// ==== 初期化 ====
// vec3(x,y,z) x:右,y:上,z:奥行　右手座標系 yaw 0が+x方向,90が+Z方向
void initCamera() {
	camera.pos = glm::vec3(0.0f, 0.0f, 0.0f);		// 位置
	camera.front = glm::vec3(0.0f, 0.0f, 1.0f);	// 前方ベクトル
	camera.up = glm::vec3(0.0f, 1.0f, 0.0f);		// 上方向ベクトル
	camera.yaw = 90.0f;	 // 初期向き
	camera.pitch = 0.0f;
}

// ==== 向きベクトル更新 ====
// Yaw/Pitchからカメラ前方向ベクトルを更新
void updateCameraFront() {
	glm::vec3 front;
	front.x = cos(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch));
	front.y = sin(glm::radians(camera.pitch));
	front.z = sin(glm::radians(camera.yaw)) * cos(glm::radians(camera.pitch));
	camera.front = glm::normalize(front);
}

// ==== カメラ適用 ====
void applyCameraView() {
	// ビュー行列を生成 glm::looat(eyeカメラ位置, centerカメラの方向 注視点, upカメラの上方向通常は0.1,0)
	// mat4は4行4列 回転、平行移動、拡大などで4*4が必要
	//	| Rx  Ux - Fx  0 |
	//	| Ry  Uy - Fy  0 |
	//	| Rz  Uz - Fz  0 |
	//	| -dot(R, pos) - dot(U, pos) dot(F, pos) 1 |	//　平行移動
	glm::mat4 view = glm::lookAt(
		camera.pos,						// 視点（カメラの位置）
		camera.pos + camera.front,		// 注視点（カメラの前方向）
		camera.up						// 上方向
	);
	// OpenGLへ適用
	glLoadMatrixf(&view[0][0]);
}
