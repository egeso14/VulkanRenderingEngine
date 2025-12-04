#pragma once
#include "VTA_game_object.h"
#include "VTA_Window.h"



namespace VTA_UI
{
	class UIController
	{
	public:
		struct KeyMappings
		{
            int select = GLFW_MOUSE_BUTTON_LEFT;
			int deselect = GLFW_MOUSE_BUTTON_RIGHT;
		};
		
		UIController();
		void SetWindow(GLFWwindow* window);
        void SelectObject(glm::mat4 view, glm::mat4 projection);
		bool GetSelectedObject(int& o_selectedId);
		bool GetClickPos(glm::vec2& clickPos);

        KeyMappings keys{};

	private:

		void RemoveSelected();
		GLFWwindow* windowRef;
		int currentlySelected = -1;

	};
}