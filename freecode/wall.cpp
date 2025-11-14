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
	g_walls.push_back(w);
}

void drawWalls()
{
	for (const auto& w : g_walls)
	{
		// 壁の中心を基準に計算
		float minX = w.pos.x - w.width * 0.5f;
		float maxX = w.pos.x + w.width * 0.5f;
		float minY = w.pos.y - w.height * 0.5f;
		float maxY = w.pos.y + w.height * 0.5f;
		float minZ = w.pos.z - w.depth * 0.5f;
		float maxZ = w.pos.z + w.depth * 0.5f;

		// 塗りつぶし
		glColor3ub(w.color.r, w.color.g, w.color.b);
		glBegin(GL_QUADS);
		// 前面
		glVertex3f(minX, minY, maxZ);
		glVertex3f(maxX, minY, maxZ);
		glVertex3f(maxX, maxY, maxZ);
		glVertex3f(minX, maxY, maxZ);
		// 背面
		glVertex3f(minX, minY, minZ);
		glVertex3f(maxX, minY, minZ);
		glVertex3f(maxX, maxY, minZ);
		glVertex3f(minX, maxY, minZ);
		// 左側面
		glVertex3f(minX, minY, minZ);
		glVertex3f(minX, minY, maxZ);
		glVertex3f(minX, maxY, maxZ);
		glVertex3f(minX, maxY, minZ);
		// 右側面
		glVertex3f(maxX, minY, minZ);
		glVertex3f(maxX, minY, maxZ);
		glVertex3f(maxX, maxY, maxZ);
		glVertex3f(maxX, maxY, minZ);
		// 上面
		glVertex3f(minX, maxY, minZ);
		glVertex3f(maxX, maxY, minZ);
		glVertex3f(maxX, maxY, maxZ);
		glVertex3f(minX, maxY, maxZ);
		// 底面
		glVertex3f(minX, minY, minZ);
		glVertex3f(maxX, minY, minZ);
		glVertex3f(maxX, minY, maxZ);
		glVertex3f(minX, minY, maxZ);
		glEnd();

		// ワイヤーフレーム
		glColor3ub(255, 255, 255);
		glLineWidth(2.0f);
		glBegin(GL_LINES);
		// 底面
		glVertex3f(minX, minY, minZ); glVertex3f(maxX, minY, minZ);
		glVertex3f(maxX, minY, minZ); glVertex3f(maxX, minY, maxZ);
		glVertex3f(maxX, minY, maxZ); glVertex3f(minX, minY, maxZ);
		glVertex3f(minX, minY, maxZ); glVertex3f(minX, minY, minZ);
		// 上面
		glVertex3f(minX, maxY, minZ); glVertex3f(maxX, maxY, minZ);
		glVertex3f(maxX, maxY, minZ); glVertex3f(maxX, maxY, maxZ);
		glVertex3f(maxX, maxY, maxZ); glVertex3f(minX, maxY, maxZ);
		glVertex3f(minX, maxY, maxZ); glVertex3f(minX, maxY, minZ);
		// 垂直辺
		glVertex3f(minX, minY, minZ); glVertex3f(minX, maxY, minZ);
		glVertex3f(maxX, minY, minZ); glVertex3f(maxX, maxY, minZ);
		glVertex3f(maxX, minY, maxZ); glVertex3f(maxX, maxY, maxZ);
		glVertex3f(minX, minY, maxZ); glVertex3f(minX, maxY, maxZ);
		glEnd();
	}
}