#include "VTA_Widget.h"





VTA_UI::VTAWidget::UiGraphNode VTA_UI::VTAWidget::createUiGraph(glm::vec2 parentMin, glm::vec2 parentMax)
{
	glm::mat4 modelMatrix;

	
	modelMatrix = rectTransform.getModel(parentMin, parentMax, isTextWidget());
	
	UiGraphNode myGraphNode;

	myGraphNode.isText = isTextWidget();
	myGraphNode.widgetId = id;
	myGraphNode.modelMatrix = modelMatrix;
	myGraphNode.widget = shared_from_this();

	glm::vec2 min = { modelMatrix[3][0], modelMatrix[3][1] }; // minimum here is the translation 
	glm::vec2 diff = { modelMatrix[0][0], modelMatrix[1][1] }; // diff here is the scale
	glm::vec2 max = min + diff;

	for (auto& child : children)
	{
		myGraphNode.children.push_back(child->createUiGraph(min, max));
	}

	return myGraphNode;
}


glm::mat4 VTA_UI::RectTransformComponent::getModel(glm::vec2 parentMin, glm::vec2 parentMax, bool isText)
{
	// all models are in a 0-1 scale. What that means is that we translate them depending on anchors and offset
	// then scale them depending on anchors and offset or size
	glm::vec2 diff = parentMax - parentMin;
	glm::vec2 startingPos = parentMin + diff * anchors.min; // in pixels
	// let's start by determining the size 

	glm::vec2 size;
	if (anchors.max == anchors.min) // then we will use size to determine size
	{
		size = anchors.size;
	}
	else
	{
		// otherwise, we will use the anchors and the space provided by our parents
		
		size = diff * (anchors.max - anchors.min);
	}

	auto transform = glm::translate(
		glm::mat4(1),
		{ startingPos.x,
		startingPos.y, 0 });
	
	/*transform = glm::rotate(transform, rotation.y, {0.f, 1.f, 0.f});
	transform = glm::rotate(transform, rotation.x, { 1.f, 0.f, 0.f });
	transform = glm::rotate(transform, rotation.z, { 0.f, 0.f, 1.f });*/

	
	transform = glm::scale(transform, { size.x, size.y, 1 }); // if is text the model will define the size 
	
	return transform;
}

glm::mat4 VTA_UI::RectTransformComponent::getProjection(float screenWidth, float screenHeight)
{

	// transform the space
	auto transform = glm::translate(glm::mat4(1), { -1, -1, 0 });
	transform = glm::scale(transform, { 2 / screenWidth, 2 / screenHeight, 1 });
	
	return transform;
}


