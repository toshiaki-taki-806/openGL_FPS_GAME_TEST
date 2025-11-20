#include "enemy.h"
#include "glut.h"

std::vector<SphereEnemy>g_enemies;

void addEnemy(const glm::vec3& pos, float radius, const glm::vec3& color)
{
	SphereEnemy e;
	e.position = pos;
	e.radius = radius;
	e.color = color;
	g_enemies.push_back(e);
}

void drawSphereEnemy()
{
	for (const auto& e : g_enemies) {
		glPushMatrix();
		glTranslatef(e.position.x, e.position.y, e.position.z);
		glColor3f(e.color.r, e.color.g, e.color.b);
		glutSolidSphere(e.radius, 24, 24);
		glPopMatrix();
	}
}