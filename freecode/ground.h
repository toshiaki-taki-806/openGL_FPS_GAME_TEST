#pragma once
#include "glm/glm.hpp"
#include "camera.h"
#include "wall.h"

extern const float GROUND_Y;

// ---- 関数プロトタイプ ----
void drawGround();
float getFloor(const std::vector<Wall>& walls);