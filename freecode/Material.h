#pragma once
#include <array>

enum class MaterialType {
	Concrete,
	Soil,
	Steel,
	Wood,
	Rubber,
	Ice,
	Glass,
	RigidBody,		// „‘Ì
	MAX
};

struct MaterialProperties {
	float restitution;  // ”½”­ŒW”
	float friction;     // –€CŒW”
};

// ‘fŞƒe[ƒuƒ‹‚ÌéŒ¾
extern const std::array<MaterialProperties, static_cast<int>(MaterialType::MAX)> g_materialTable;