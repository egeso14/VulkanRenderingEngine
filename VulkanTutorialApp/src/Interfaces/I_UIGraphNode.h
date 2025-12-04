#pragma once

#include "glm/glm.hpp"
#include <memory>


namespace VTA_UI
{
	class VTAWidget;
	struct Anchors;

	class I_UIGraphNode
	{
	public:
		virtual ~I_UIGraphNode() = default;

		virtual void AddChild(std::shared_ptr<I_UIGraphNode> newChild) = 0;
		virtual std::shared_ptr<VTAWidget> GetWidget() = 0;
		virtual std::shared_ptr<VTAWidget> GetLeafAtPos(Anchors parentAnchors, glm::vec2 screenPos, glm::vec2 pivots = { 0, 0 }) = 0;// pivots on relevant for fixed size
		
	};
}
