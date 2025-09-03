#pragma once
#include "FlatMesh.h"
#include <memory>
#include <unordered_map>

namespace VTA_UI
{
	struct UiGraphNode
	{
		bool isText;
		glm::mat4 modelMatrix;
		std::shared_ptr<FlatMesh> model;
		std::vector<UiGraphNode> children;
	};

	struct RectTransformComponent
	{
		glm::vec3 translation{};
		glm::vec3 scale{ 1.f, 1.f, 1.f };
		glm::vec3 rotation{};

		glm::mat4 mat4();
	};

	class VTAWidget {
	public:

		// delete copy constuctor and assignment operator because we want to avoid having duplicate game objects
		VTAWidget(const VTAWidget&) = delete;
		VTAWidget& operator=(const VTAWidget&) = delete;
		VTAWidget(VTAWidget&&) = default;
		VTAWidget& operator=(VTAWidget&&) = default;

		void AddChild(std::unique_ptr<VTAWidget> newChild);
		UiGraphNode createUiGraph();

		glm::vec3 color{};
		RectTransformComponent rectTransform{};
		std::shared_ptr<FlatMesh> model;
		std::vector<std::unique_ptr<VTAWidget>> children;

	


	};
}

