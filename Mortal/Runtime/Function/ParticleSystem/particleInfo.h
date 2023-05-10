#pragma once 
#include "Math/math.h"
namespace mortal
{
	struct ParticlesInfo
	{
		alignas(0) glm::vec3 Position; 
		alignas(16) glm::vec3 Velocity; 
	};
    
    
} // namespace mortal
