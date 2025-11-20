#pragma once
#include "glut.h"
#include "glm/glm.hpp"
#include <vector>

struct SphereEnemy {
	glm::vec3 position;
	float radius;
	glm::vec3 color;
};
extern std::vector<SphereEnemy> g_enemies;

void addEnemy(const glm::vec3& pos, float radius, const glm::vec3& color);
void drawSphereEnemy();
