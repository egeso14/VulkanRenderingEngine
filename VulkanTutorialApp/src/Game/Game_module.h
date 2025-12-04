#pragma once
#include <unordered_map>
#include "VTA_frame_info.h"
#include "VTA_buffer.h"
#include "VTA_game_object.h"
#include "keyboard_movement_controller.h"
#include "VTA_camera.h"


namespace VTA
{
	class Game_Module
	{
	public:
		Game_Module(VTA::ResourceLoader& loader, std::string rendererName, VTA::VTADevice& device, int framesInFlight);
		FrameInfo Update(GLFWwindow* window,
			float frameTime, float aspect, float frameIndex,
			glm::vec2 screenDims,
			VkCommandBuffer commandBuffer);
		static VTAGameObject::Map& GetGameObjects();
		ResourceSet GetGlobalResourceSet();

		void BeginFrame(uint32_t frameIndex);
		void EndFrame();

		static glm::mat4 GetViewMatrix();
		static glm::mat4 GetProjectionMatrix();
		

	private:
		std::unordered_map<std::string, std::shared_ptr<VTAModel>> models;
		std::unordered_map<std::string, Resource*> resources;

		void LoadResources();
		void LoadModels();
		void LoadGameObjects();
		void LoadLights();
		void UpdateLights(FrameInfo frameInfo, GlobalUbo& ubo);
		void CreateCommandBuffers();

		VkCommandBuffer currentCommandBuffer;
		std::vector<VkCommandBuffer> commandBuffers;
		std::unique_ptr<ResourceSet> globalSet;
		VTABuffer* globalUboBuffer;
		VTADevice& device;
		static VTAGameObject::Map* gameObjects;
		static VTACamera camera;
		VTAGameObject viewerObject;
		KeyboardMovementController cameraController{};
		ResourceLoader& loader;
		std::string rendererName;
		int framesInFlight;
	};
}