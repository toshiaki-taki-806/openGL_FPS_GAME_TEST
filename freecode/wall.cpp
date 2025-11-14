#include "wall.h"

static std::vector<Wall> g_walls;

void addWall(const glm::vec3& pos, float width, float height, float depth, const glm::vec3& color)
{
	g_walls.push_back({ pos, width, height, depth, color });
}

void drawWalls()
{
	for (const auto& w : g_walls)
	{
		float minX = w.pos.x - w.width * 0.5f;
		float maxX = w.pos.x + w.width * 0.5f;
		float minY = w.pos.y;
		float maxY = w.pos.y + w.height;
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