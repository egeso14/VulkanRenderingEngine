#include "VTA_Widget.h"


void VTA_UI::VTAWidget::AddChild(std::unique_ptr<VTAWidget> newChild)
{
	children.push_back(newChild);
}

void VTA_UI::VTAWidget::CreateUiGraph(glm::mat4 parentTransform)
{

}
