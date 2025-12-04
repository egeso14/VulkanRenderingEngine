#include "UIController.h"
#include "SpatialUtilities.h"
#include <iostream>

namespace VTA_UI
{
	VTA_UI::UIController::UIController()
	{
	}

	void UIController::SetWindow(GLFWwindow* window)
	{
		windowRef = window;
	}

	void VTA_UI::UIController::SelectObject(glm::mat4 view, glm::mat4 projection)
	{
		// will the selected object change?
		if (glfwGetMouseButton(windowRef, keys.deselect) == GLFW_PRESS)
		{
			if (currentlySelected != -1)
			{
				RemoveSelected();
			}

			currentlySelected = -1;
		}
		else if (glfwGetMouseButton(windowRef, keys.select) == GLFW_PRESS)
		{
			double mouseX, mouseY;
			glfwGetCursorPos(windowRef, &mouseX, &mouseY);
			glm::vec2 screenPos = { mouseX, mouseY };
			std::cout << "x coordinate is " << std::to_string(mouseX) << " || y coordinate is " << std::to_string(mouseY) << "\n";
			auto worldSpaceRay = ScreenSpaceToWorldSpaceRay(view, projection, screenPos, VTA::VTARenderer::getScreenWidth(), VTA::VTARenderer::getScreenHeight());
			auto hitResults = GetRaycastHit(worldSpaceRay);
			std::string name = hitResults.objName;
			if (hitResults.hit)
			{
				if (currentlySelected != -1)
				{
					RemoveSelected();
				}

				currentlySelected = hitResults.objectId;
				auto& objects = VTA::Game_Module::GetGameObjects();
				auto& selectedObject = objects.find(currentlySelected)->second;
				selectedObject.Select();

			}

		
		}

		
	}
	bool UIController::GetSelectedObject(int& o_selectedId)
	{
		if (currentlySelected == -1) return false;

		o_selectedId = currentlySelected;
		return true;
	}
	bool UIController::GetClickPos(glm::vec2& clickPos)
	{
		if (glfwGetMouseButton(windowRef, keys.select) == GLFW_PRESS)
		{
			double mouseX, mouseY;
			glfwGetCursorPos(windowRef, &mouseX, &mouseY);
			clickPos = { mouseX, mouseY };
			return true; 
		}
		return false;
	}
	void UIController::RemoveSelected()
	{
		auto& objects = VTA::Game_Module::GetGameObjects();
		auto& selectedObject = objects.find(currentlySelected)->second;

		selectedObject.Deselect();
	}
}

