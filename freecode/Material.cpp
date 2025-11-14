#include "Material.h"

const std::array<MaterialProperties, static_cast<int>(MaterialType::MAX)> g_materialTable = {
	MaterialProperties{0.05f, 0.8f},   // Concrete
	MaterialProperties{0.02f, 0.9f},   // Soil
	MaterialProperties{0.1f, 0.6f},    // Steel
	MaterialProperties{0.2f, 0.5f},    // Wood
	MaterialProperties{0.8f, 0.9f},    // Rubber
	MaterialProperties{0.01f, 0.02f},  // Ice
	MaterialProperties{0.05f, 0.4f},   // Glass
	MaterialProperties{0.95f, 0.01f},  // RigidBody
};