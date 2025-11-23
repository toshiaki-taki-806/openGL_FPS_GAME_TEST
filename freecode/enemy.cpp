#include "enemy.h"
#include "glut.h"

std::vector<SphereEnemy>g_enemies;

void addEnemy(const glm::vec3& _pos, const glm::vec3& _color, float _radius, int _life)
{
	SphereEnemy e;
	e.position = _pos;
	e.color = _color;
	e.radius = _radius;
	e.life = _life;
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