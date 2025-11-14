#pragma once
#include <vector>
#include "glut.h"
#include "glm/glm.hpp"
#include "Material.h"

struct Wall {
	glm::vec3 pos;    // íÜêSà íu
	float width;
	float height;
	float depth;
	glm::vec3 color;  // RGB
};

void addWall(const glm::vec3& pos, float width, float height, float depth, const glm::vec3& color);
void drawWalls();
