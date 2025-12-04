#pragma once
#include "VTA_model.h"

// libs 
#include <glm/gtc/matrix_transform.hpp>


// std
#include <memory>
#include <unordered_map>
#include "Resource.h"


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

	struct AABB
	{
		glm::vec3 min;
		glm::vec3 max;
	};

	class ColliderComponent
	{

	};


	struct PointLightComponent
	{
		float lightIntensity = 1.0f;
	};

	struct OutlineComponent
	{
		glm::vec4 color;
		float width;
	};

	class VTAGameObject
	{
	public:
	using id_t = unsigned int;
	using Map = std::unordered_map<id_t, VTAGameObject>;

	void Select();
	void Deselect();

	static VTAGameObject makePointLight(float intensity = 10.f, float radius = 0.1f, glm::vec3 color = glm::vec3(1.f));
	std::vector<id_t> Children();


	const AABB GetBoundingBox();
	void AddBoundingBox(glm::vec3 extents);

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

	std::shared_ptr<PointLightComponent> pointLight = nullptr;
	std::shared_ptr<OutlineComponent> outlineComponent = nullptr;
	std::shared_ptr<VTAModel> model{};
	std::unique_ptr<VTA::ResourceSet> resources;
	glm::vec3 color{};
	std::string name;

	// components

	TransformComponent transform{};


	private:
		VTAGameObject(id_t objId) : id(objId) {}

		id_t id;
		AABB* boundingBox;
	};


}