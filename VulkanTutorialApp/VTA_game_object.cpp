#include "VTA_game_object.h"

namespace VTA
{
	glm::mat4 TransformComponent::mat4() {

		auto transform = glm::translate(glm::mat4(1.f), translation);

		transform = glm::rotate(transform, rotation.y, { 0.f, 1.f, 0.f });
		transform = glm::rotate(transform, rotation.x, { 1.f, 0.f, 0.f });
		transform = glm::rotate(transform, rotation.z, { 0.f, 0.f, 1.f });

		transform = glm::scale(transform, scale);
		return transform;
	}

	glm::mat3 TransformComponent::normalMatrix() {
		return glm::transpose(glm::inverse(glm::mat3(mat4())));
	}
	VTAGameObject VTAGameObject::makePointLight(float intensity, float radius, glm::vec3 color)
	{
		VTAGameObject gameObj = VTAGameObject::createGameObject();
		gameObj.color = color;
		gameObj.pointLight = std::make_unique<PointLightComponent>();
		gameObj.transform.scale.x = radius;
		gameObj.pointLight->lightIntensity = intensity;
		return gameObj;
	}
}