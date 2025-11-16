#pragma once
#include <vector>
#include "glut.h"
#include "glm/glm.hpp"
#include "Material.h"

struct Wall {
	glm::vec3 pos;		// 中心位置
	float width;		// 幅
	float height;		// 高さ
	float depth;		// 奥行
	glm::vec3 color;	// RGB
	float friction;		// 摩擦係数
	float restitution;	// 反発係数
	glm::vec3 AABBmin;
	glm::vec3 AABBmax;

	MaterialType material;		// 材質タイプ
};
extern std::vector<Wall> g_walls; // 他の cpp からも参照可能


void addWall(const glm::vec3& pos,
	float width,
	float height,
	float depth,
	const glm::vec3& color,
	MaterialType material);

void addWall(const glm::vec3& pos, float width, float height, float depth, const glm::vec3& color, MaterialType material);
void drawWalls();
