#pragma once
#include "FlatMesh.h"
#include <memory>
#include <unordered_map>
#include <stdexcept>
#include "VTA_device.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace VTA_UI
{
	// two types of anchors: single point and stretch
	// in fractions of the screen dimensions
	// coordinate system centered on top left of the screen
	struct Anchors
	{
		glm::vec2 min;
		glm::vec2 max;
		glm::vec2 size;// if min and max are the same we use pixel sizes
	};

	struct Offsets
	{
		float top;
		float left;
		float bottom;
		float right;
	};

	struct RectTransformComponent
	{
		glm::vec2 pivots;
		Anchors anchors;
		Offsets offsets;
		glm::vec3 scale{ 1.f, 1.f, 1.f };
		glm::vec3 rotation{};

		glm::mat4 getModel(glm::vec2 parentMin, glm::vec2 parentMax, bool isText);
		glm::mat4 getProjection(float screenWidth, float screenHeight);
	};

	class VTAWidget: public std::enable_shared_from_this<VTAWidget> {
	public:
		using id_t = unsigned int;
	private:
		id_t id;
		
		bool isTopLevel;
		std::list<std::shared_ptr<VTAWidget>>::iterator topLevelIterator{};
	
	public:
		VTAWidget(id_t newId) { id = newId; }
		inline static std::list<std::shared_ptr<VTAWidget>> topLevelWidgets{};
		glm::vec3 color{};
		RectTransformComponent rectTransform{};
		std::shared_ptr<FlatMesh> model;
		std::vector<std::shared_ptr<VTAWidget>> children;

		struct UiGraphNode
		{
			bool isText;
			glm::mat4 modelMatrix;
			id_t widgetId;
			std::shared_ptr<VTAWidget> widget; // find to have a ref here since it is temporary, maybe change it to a weak pointer??
			std::vector<UiGraphNode> children;
		};

		static const std::list<std::shared_ptr<VTAWidget>>& getTopLevelWidgets() { return topLevelWidgets; }

		// delete copy constuctor and assignment operator because we want to avoid having duplicate game objects
		VTAWidget(const VTAWidget&) = delete;
		VTAWidget& operator=(const VTAWidget&) = delete;
		VTAWidget(VTAWidget&&) = default;
		VTAWidget& operator=(VTAWidget&&) = default;


		// creates a shared pointer and saves a copy of it within the registry map and top level widgets
		static std::shared_ptr<VTAWidget> create()
		{
			static id_t current_id = 0;
			
			auto sharedPointer = std::make_shared<VTAWidget>(current_id);

			topLevelWidgets.push_back(sharedPointer);

			sharedPointer->isTopLevel = true;
			sharedPointer->topLevelIterator = std::prev(topLevelWidgets.end());
			return std::move(sharedPointer);
		}
		
		// makes sure that the passed in shared pointer only has 1 owner and then deletes it and all its children
		// lastly removes itself from the registries and deletes the shared pointer
		static void destroy(std::shared_ptr<VTAWidget> widget)
		{
			if (widget.use_count() > 1)
			{
				throw std::runtime_error("other shared pointers exist to this object");
			}

			for (std::shared_ptr<VTAWidget>& child : widget->children)
			{
				VTAWidget::destroy(std::move(child));
			}
			
			if (widget->isTopLevel)
			{
				topLevelWidgets.erase(widget->topLevelIterator);
			}


		}


		void AddChild(std::shared_ptr<VTAWidget> newChild)
		{
			if (newChild.use_count() > 2)
			{
				throw std::runtime_error("other shared pointers exist to this object");
			}

			if (newChild->isTopLevel)
			{
				newChild->isTopLevel = false;
				topLevelWidgets.erase(newChild->topLevelIterator);
			}

			children.push_back(std::move(newChild));
			
		}
		
		UiGraphNode createUiGraph(glm::vec2 parentMin, glm::vec2 parentMax);
		id_t getId() { return id; }
		
		bool isTextWidget() { return model->isTextMesh; }
		bool getIsTopLevel() { return isTopLevel; }

		



		

	
	};
}

