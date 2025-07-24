#pragma once
#include "VTA_model.h"

// std
#include <memory>


namespace VTA
{
	struct Transform2dComponent
	{
		glm::vec2 translation {};
		glm::vec2 scale{ 1.f, 1.f };
		float rotation;

		glm::mat2 mat2() {

			const float s = glm::sin(rotation);
			const float c = glm::cos(rotation);

			glm::mat2 rotationMat{ { c, s } , { -s, c } }; // these are columns!!!!
			glm::mat2 scaleMat{ {scale.x, 0.f}, {0.f, scale.y} }; // these are columns!!!!
			return rotationMat * scaleMat;
		}
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

	Transform2dComponent transform2d{};


	private:
		VTAGameObject(id_t objId) : id(objId) {}

		id_t id;
	};
}