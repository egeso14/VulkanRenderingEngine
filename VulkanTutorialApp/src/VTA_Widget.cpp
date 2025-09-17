#include "VTA_Widget.h"





VTA_UI::VTAWidget::UiGraphNode VTA_UI::VTAWidget::CreateUiGraph(glm::mat4 parentTransform)
{
	glm::mat4 modelMatrix = parentTransform * rectTransform.mat4();

	UiGraphNode myGraphNode;

	myGraphNode.isText = isTextWidget();
	myGraphNode.widgetId = id;
	myGraphNode.modelMatrix = modelMatrix;

	for (auto& child : children)
	{
		myGraphNode.children.push_back(child->CreateUiGraph(modelMatrix));
	}

	return myGraphNode;
}

glm::mat4 VTA_UI::RectTransformComponent::mat4() {

	auto transform = glm::translate(glm::mat4(1.f), translation);

	transform = glm::rotate(transform, rotation.y, { 0.f, 1.f, 0.f });
	transform = glm::rotate(transform, rotation.x, { 1.f, 0.f, 0.f });
	transform = glm::rotate(transform, rotation.z, { 0.f, 0.f, 1.f });

	transform = glm::scale(transform, scale);
	return transform;
}


