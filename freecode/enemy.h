#pragma once
#include "glut.h"
#include "glm/glm.hpp"
#include <vector>

struct SphereEnemy {
	glm::vec3 position;
	glm::vec3 color;
	float radius;
	int life;
};
extern std::vector<SphereEnemy> g_enemies;

void addEnemy(const glm::vec3& _pos, const glm::vec3& _color, float _radius, int _life);
void drawSphereEnemy();
