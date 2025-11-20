#include "enemy.h"
#include "glut.h"

std::vector<SphereEnemy> EnemyManager::g_enemies;

void EnemyManager::addEnemy(const glm::vec3& pos, float radius, const glm::vec3& color)
{
	SphereEnemy e(pos, radius, color);
	g_enemies.push_back(e);
}

void EnemyManager::drawSphereEnemy()
{
	for (const auto& e : g_enemies) {
		glPushMatrix();
		glTranslatef(e.position.x, e.position.y, e.position.z);
		glColor3f(e.color.r, e.color.g, e.color.b);
		glutSolidSphere(e.radius, 24, 24);
		glPopMatrix();
	}
}