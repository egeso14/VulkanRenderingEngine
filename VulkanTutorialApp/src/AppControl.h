#pragma once

#include "VTA_Window.h"
#include "VTA_device.hpp"
#include "VTA_renderer.h"
#include "VTA_model.h"
#include "VTA_game_object.h"
#include "VTA_descriptors.h"
#include "VTA_Widget.h"
#include "Trex/Atlas.hpp"

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

		void loadWidgetObjects();
		void loadGameObjects();
		void createFontAtlas();


		Trex::Atlas* fontAtlas;
		float MAX_FRAME_TIME{ 0.2f };
		VTAWindow window{ WIDTH, HEIGHT, "Vulkan Window" };
		VTADevice device{ window };
		VTARenderer renderer{ window, device };

		std::vector<VTADescriptorAllocatorGrowable> descriptorAllocators;
		std::vector<VkDescriptorSet> globalDescriptorSets;
		std::vector<VkDescriptorSet> editorUIDescriptorSets;
		
		VTAGameObject::Map gameObjects;
		std::vector<std::shared_ptr<VTA_UI::VTAWidget>> widgets;
		
	};
}