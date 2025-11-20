#include "glut.h"
#include "camera.h"
#include "ground.h"

void drawGround()
{
	const int viewRange = 20;     // カメラから描画する範囲（半径20m）
	const float tileSize = 1.0f;  // 1タイルの幅・奥行き（1m四方）

	// カメラの現在座標を整数に丸める（タイル単位で扱うため）
	int camX = static_cast<int>(camera.pos.x);
	int camZ = static_cast<int>(camera.pos.z);

	glColor3f(1.0f, 1.0f, 1.0f);  // 線の色は白
	glLineWidth(1.0f);            // 線幅

	glBegin(GL_LINES);
	// X方向の線
	for (int x = camX - viewRange; x <= camX + viewRange; ++x)
	{
		float xf = x * tileSize;
		float zStart = (camZ - viewRange) * tileSize;
		float zEnd = (camZ + viewRange + 1) * tileSize;
		glVertex3f(xf, 0.0f, zStart);
		glVertex3f(xf, 0.0f, zEnd);
	}

	// Z方向の線
	for (int z = camZ - viewRange; z <= camZ + viewRange; ++z)
	{
		float zf = z * tileSize;
		float xStart = (camX - viewRange) * tileSize;
		float xEnd = (camX + viewRange + 1) * tileSize;
		glVertex3f(xStart, 0.0f, zf);
		glVertex3f(xEnd, 0.0f, zf);
	}
	glEnd();
}

float getFloor(const std::vector<Wall>& walls) {
	float maxFloorY = GROUND_Y;
	//const float STEP_HEIGHT = 0.2f;		// 段差を乗り越える処理

	float footY = player.footPos.y; // 足元のY（プレイヤーの底）
	//float maxCandidateY = footY + STEP_HEIGHT; // 足元からステップまで

	for (const auto& w : g_walls) {
		// キャラクターのXZが壁の範囲内なら候補
		if (camera.pos.x + player.radius >= w.AABBmin.x && camera.pos.x - player.radius <= w.AABBmax.x &&
			camera.pos.z + player.radius >= w.AABBmin.z && camera.pos.z - player.radius <= w.AABBmax.z)
		{
			// 上面を取得
			float topY = w.AABBmax.y;
			// 足元〜ステップ高さ内なら floorY 候補
			//if (topY >= footY && topY <= maxCandidateY) {
			if (topY <= footY) {
				if (topY > maxFloorY)
					maxFloorY = topY;
			}
		}	
	}
	return maxFloorY;
}