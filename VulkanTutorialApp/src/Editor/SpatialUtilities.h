#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include "VTA_renderer.h"
#include "Game_module.h"
#include <iostream>

struct Ray
{ 
	glm::vec3 startPos;
	glm::vec3 direction;
};

Ray ScreenSpaceToWorldSpaceRay(
	glm::mat4 viewMatrix, 
	glm::mat4 projectionMatrix,
	glm::vec2 screenCoordinates,
	float screenWidth, float screenHeight)
{
	// we start by normalizing our screen coordinates to fit into the -1 to 1 range
	auto normalized_x = 2 * screenCoordinates.x / screenWidth - 1;
	auto normalized_y = 2 * screenCoordinates.y / screenHeight - 1;

	
	glm::vec4 ray_clip = glm::vec4(normalized_x,normalized_y, 1, 1);

	// now let's unproject, starting first with the projection matrix

	glm::vec4 rayEye = glm::inverse(projectionMatrix) * ray_clip;
	rayEye = glm::vec4(rayEye.x, rayEye.y, 1, 0);
	auto inverseView = glm::inverse(viewMatrix);
	glm::vec3 cameraPos = { inverseView[3][0], inverseView[3][1], inverseView[3][2] };
	glm::vec4 rayWorldHomo = glm::inverse(viewMatrix) * rayEye;
	glm::vec3 rayWorld = { rayWorldHomo.x, rayWorldHomo.y, rayWorldHomo.z };
	rayWorld = glm::normalize(rayWorld) * glm::vec3(1, 1, 1);

	std::cout << "The ray created for this point on the screen is: " << "X: " << rayWorld.x << " "
		<< "Y: " << rayWorld.y << " "
		<< "Z: " << rayWorld.z << " ";

	

	return Ray{cameraPos, rayWorld};

}

struct Hit {
	bool        hit = false;
	float       t = std::numeric_limits<float>::infinity();
	glm::vec3   point{ 0 };
	glm::vec3   normal{ 0 };
	uint64_t    objectId = 0;                      
	std::string objName;
};

// Slab method: returns true if ray intersects, with param range [tEnter, tExit]
inline bool RayAABB(const Ray& r, const VTA::AABB& b, float& tEnter, float& tExit)
{
	const glm::vec3 invD = 1.0f / r.direction;              // OK if a component is 0 ? ±inf
	const glm::vec3 t0 = (b.min - r.startPos) * invD;
	const glm::vec3 t1 = (b.max - r.startPos) * invD;

	const glm::vec3 tmin = glm::min(t0, t1);
	const glm::vec3 tmax = glm::max(t0, t1);

	tEnter = std::max(std::max(tmin.x, tmin.y), tmin.z);
	tExit = std::min(std::min(tmax.x, tmax.y), tmax.z);

	// Hit if the interval is valid and intersects ray forward half-line
	return tExit >= std::max(tEnter, 0.0f);
}


Hit GetRaycastHit(Ray ray)
{
	// assuming for now that all objects have AABBs
	auto& gameObjects = VTA::Game_Module::GetGameObjects();
	Hit currentClosestHit = { false };
	float minT = 100000;

	for (auto itr = gameObjects.begin(); itr != gameObjects.end(); ++itr)
	{
		auto key = itr->first;
		auto& object = itr->second;
		auto boundingBox = object.GetBoundingBox();

		float tEnter, tExit;
		if (RayAABB(ray, boundingBox, tEnter, tExit))
		{
			if (tEnter > 0 && minT > tEnter)
			{
				minT = tEnter;
				auto hitPos = ray.startPos + minT * ray.direction;
				currentClosestHit = { true, minT, hitPos, glm::vec3(0, 0, 0),itr->first, object.name};
			}
		}

	}
	return currentClosestHit;
}

