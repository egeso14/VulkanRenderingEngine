#pragma once

#include "VTA_Window.h"
#include "VTA_device.hpp"
#include "VTA_renderer.h"
#include "VTA_model.h"
#include "VTA_game_object.h"

#include <memory>
#include <vector>

namespace VTA
{
	class AppControl
	{
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;


		AppControl();
		~AppControl();

		AppControl(const AppControl&) = delete;
		AppControl& operator=(const AppControl&) = delete; // this is to establish unique ownership of resources

		void run();
	private:

		void loadGameObjects();


		VTAWindow window{ WIDTH, HEIGHT, "Vulkan Window" };
		VTADevice device{ window };
		VTARenderer renderer{ window, device };
		std::vector<VTAGameObject> gameObjects;
	};
}