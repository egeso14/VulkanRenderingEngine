#pragma once
#include "VTA_model.h"

// libs 
#include <glm/gtc/matrix_transform.hpp>


// std
#include <memory>


namespace VTA
{
	struct TransformComponent
	{
		glm::vec3 translation{};
		glm::vec3 scale{ 1.f, 1.f, 1.f };
		glm::vec3 rotation{};

		// Matrix corresponds to translate * Ry *Rx * Rz * scale transformation
		glm::mat4 mat4();
		glm::mat3 normalMatrix();
	};


	class VTAGameObject
	{
	public:
	using id_t = unsigned int;


	// delete copy constuctor and assignment operator because we want to avoid having duplicate game objects
	VTAGameObject(const VTAGameObject&) = delete;
	VTAGameObject& operator=(const VTAGameObject&) = delete;
	VTAGameObject(VTAGameObject&&) = default;
	VTAGameObject& operator=(VTAGameObject&&) = default;


	static VTAGameObject createGameObject()
	{
		static id_t current_id = 0;
		return VTAGameObject{ current_id++ };
	}

	id_t getId() const { return id; }


	std::shared_ptr<VTAModel> model{};
	glm::vec3 color{};

	// components

	TransformComponent transform{};


	private:
		VTAGameObject(id_t objId) : id(objId) {}

		id_t id;
	};
}