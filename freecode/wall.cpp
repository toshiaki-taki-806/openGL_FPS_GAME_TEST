#include <vector>
#include "glut.h"
#include "Material.h"
#include "wall.h"

std::vector<Wall> g_walls;

void addWall(const glm::vec3& pos,
	float width,
	float height,
	float depth,
	const glm::vec3& color,
	MaterialType material)
{
	Wall w;
	w.pos = pos;
	w.width = width;
	w.height = height;
	w.depth = depth;
	w.color = color;
	w.material = material;
	w.friction = g_materialTable[static_cast<int>(material)].friction;
	w.restitution = g_materialTable[static_cast<int>(material)].restitution;

	// --- AABB を計算して代入 ---
	w.AABBmin = glm::vec3(
		pos.x - width * 0.5f,
		pos.y - height * 0.5f,
		pos.z - depth * 0.5f
	);
	w.AABBmax = glm::vec3(
		pos.x + width * 0.5f,
		pos.y + height * 0.5f,
		pos.z + depth * 0.5f
	);
	g_walls.push_back(w);
}

void drawWalls()
{
	for (const auto& w : g_walls)
	{
		// 壁の中心を基準に計算
		const glm::vec3& min = w.AABBmin;
		const glm::vec3& max = w.AABBmax;

		// 塗りつぶし
		glColor3ub(w.color.r, w.color.g, w.color.b);
		glBegin(GL_QUADS);

		// 前面
		glVertex3f(min.x, min.y, max.z);
		glVertex3f(max.x, min.y, max.z);
		glVertex3f(max.x, max.y, max.z);
		glVertex3f(min.x, max.y, max.z);

		// 背面
		glVertex3f(min.x, min.y, min.z);
		glVertex3f(max.x, min.y, min.z);
		glVertex3f(max.x, max.y, min.z);
		glVertex3f(min.x, max.y, min.z);

		// 左側面
		glVertex3f(min.x, min.y, min.z);
		glVertex3f(min.x, min.y, max.z);
		glVertex3f(min.x, max.y, max.z);
		glVertex3f(min.x, max.y, min.z);

		// 右側面
		glVertex3f(max.x, min.y, min.z);
		glVertex3f(max.x, min.y, max.z);
		glVertex3f(max.x, max.y, max.z);
		glVertex3f(max.x, max.y, min.z);

		// 上面
		glVertex3f(min.x, max.y, min.z);
		glVertex3f(max.x, max.y, min.z);
		glVertex3f(max.x, max.y, max.z);
		glVertex3f(min.x, max.y, max.z);

		// 底面
		glVertex3f(min.x, min.y, min.z);
		glVertex3f(max.x, min.y, min.z);
		glVertex3f(max.x, min.y, max.z);
		glVertex3f(min.x, min.y, max.z);

		glEnd();

		// ワイヤーフレーム
		glColor3ub(255, 255, 255);
		glLineWidth(2.0f);
		glBegin(GL_LINES);
		// 底面
		glVertex3f(min.x, min.y, min.z); glVertex3f(max.x, min.y, min.z);
		glVertex3f(max.x, min.y, min.z); glVertex3f(max.x, min.y, max.z);
		glVertex3f(max.x, min.y, max.z); glVertex3f(min.x, min.y, max.z);
		glVertex3f(min.x, min.y, max.z); glVertex3f(min.x, min.y, min.z);
		// 上面
		glVertex3f(min.x, max.y, min.z); glVertex3f(max.x, max.y, min.z);
		glVertex3f(max.x, max.y, min.z); glVertex3f(max.x, max.y, max.z);
		glVertex3f(max.x, max.y, max.z); glVertex3f(min.x, max.y, max.z);
		glVertex3f(min.x, max.y, max.z); glVertex3f(min.x, max.y, min.z);
		// 垂直辺
		glVertex3f(min.x, min.y, min.z); glVertex3f(min.x, max.y, min.z);
		glVertex3f(max.x, min.y, min.z); glVertex3f(max.x, max.y, min.z);
		glVertex3f(max.x, min.y, max.z); glVertex3f(max.x, max.y, max.z);
		glVertex3f(min.x, min.y, max.z); glVertex3f(min.x, max.y, max.z);
		glEnd();
	}
}