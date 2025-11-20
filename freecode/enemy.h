#pragma once
#include "glut.h"
#include "glm/glm.hpp"
#include <vector>

class SphereEnemy {
public:
	glm::vec3 position;
	float radius;
	glm::vec3 color;

	SphereEnemy(const glm::vec3& pos, float r, const glm::vec3& col)
		: position(pos), radius(r), color(col) {
	}

	void drawSphereEnemy();
};

// ìGä«óùóp
namespace EnemyManager {
	extern std::vector<SphereEnemy> g_enemies;

	void addEnemy(const glm::vec3& pos, float radius, const glm::vec3& color);
	void drawSphereEnemy();
};