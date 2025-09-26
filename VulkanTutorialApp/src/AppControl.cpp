#include "AppControl.h"
#include "simple_render_system.h"
#include "VTA_camera.h"
#include "keyboard_movement_controller.h"
#include "point_light_system.h"
#include "VTA_Buffer.h"
#include "VTA_image.h"
#include <stdexcept>
#include <array>
#include <chrono>
#include <numeric>
#include <filesystem>
#include "VTA_editor_ui_render_system.h"
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>



#define STBI_MSC_SECURE_CRT
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE


#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_TRUETYPE_IMPLEMENTATION   // default if not set by build
#include "stb_truetype.h"


namespace fs = std::filesystem;
namespace VTA
{ 


	

	AppControl::AppControl()
	{
		createFontAtlas();
		loadGameObjects(); // load the model data into memory
		loadWidgetObjects();
		
	}

	AppControl::~AppControl()
	{
		for (int i = 0; i < descriptorAllocators.size(); i++)
		{
			descriptorAllocators[i].clear_pools(device.device());
			descriptorAllocators[i].destroy_pools(device.device());
		}

		//delete fontAtlas;
	}

	void AppControl::run()
	{
		
		


		auto minOffsetAllignment = std::lcm(device.properties.limits.minUniformBufferOffsetAlignment, device.properties.limits.nonCoherentAtomSize);

		VTABuffer globalUboBuffer{ device, sizeof(GlobalUbo), VTASwapChain::MAX_FRAMES_IN_FLIGHT,  // how many uniform buffer objects in our uniform buffer? one for each frame in flight
									VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, // double buffering like this should be used whenever we have a synamic resource that is written to each frame
									minOffsetAllignment};

		globalUboBuffer.map(); // map the buffer to host memory so we can write to it

		auto globalSetLayout = VTADescriptorSetLayout::Builder(device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();

		auto uiSetLayout = VTADescriptorSetLayout::Builder(device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();

		descriptorAllocators = std::vector<VTADescriptorAllocatorGrowable>(VTASwapChain::MAX_FRAMES_IN_FLIGHT);

		VTA_Image::Texture testTexture(device, "../Textures/OnyxTexture4K.jpg");

		//FontAtlas fontAtlas;
		//BuildFontAtlas("fonts\Inter_default.ttf", 32, 32, 95, 512, 512,fontAtlas);
		VTA_Image::Texture fontAtlasTexture(device, *fontAtlas);
		
		for (int i = 0; i < descriptorAllocators.size(); i++)
		{
			std::vector < VTADescriptorAllocatorGrowable::PoolSizeRatio> frame_sizes = {
					{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3 },
					{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3 },
					{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3 },
					{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4 },
			};


			descriptorAllocators[i] = VTADescriptorAllocatorGrowable{};
			descriptorAllocators[i].init(device.device(), 1000, frame_sizes);
			auto bufferInfo = globalUboBuffer.descriptorInfo(); // get the descriptor info for the uniform buffer
			auto imageInfo = testTexture.descriptorInfo();
			auto fontImageInfo = fontAtlasTexture.descriptorInfo();

			VkDescriptorSet globalDescriptorSet = descriptorAllocators[i].allocate( // allocate a descriptor set from the descriptor allocator
				device.device(),
				globalSetLayout->getDescriptorSetLayout()
			);

			VkDescriptorSet editorUIDescriptorSet = descriptorAllocators[i].allocate(
				device.device(),
				uiSetLayout->getDescriptorSetLayout()
			);

			VTADescriptorWriter writer{ *globalSetLayout };
			writer.writeBuffer(0, &bufferInfo); // write to the writes vector
			writer.writeImage(1, &imageInfo);
			writer.overwrite(globalDescriptorSet, device); // bind descriptor sets to the buffers in the writes vector

			VTADescriptorWriter fontWriter { *uiSetLayout };
			fontWriter.writeImage(0, &fontImageInfo);
			fontWriter.overwrite(editorUIDescriptorSet, device);

			globalDescriptorSets.push_back(globalDescriptorSet);
			editorUIDescriptorSets.push_back(editorUIDescriptorSet);
		}

		SimpleRenderSystem simpleRenderSystem{ device, renderer.getSwapChainRenderPass1(), globalSetLayout->getDescriptorSetLayout()}; // create the render system with the device and the swap chain render pass
		PointLightSystem pointLightSystemSystem{ device, renderer.getSwapChainRenderPass1(), globalSetLayout->getDescriptorSetLayout() }; // create the render system with the device and the swap chain render pass

		VTA_UI::EditorUIRenderSystem editorUIRenderSystem{ device, renderer.getSwapChainRenderPass2(), uiSetLayout->getDescriptorSetLayout() };

		

		

        VTACamera camera{};
        //camera.setViewDirection(glm::vec3(0.f), glm::vec3(0.5f, 0.f, 1.f));
        
        auto viewerObject = VTAGameObject::createGameObject();
		viewerObject.transform.translation = { 0.f, 0.f, -2.5f }; 
        KeyboardMovementController cameraController{};
        
        camera.setViewTarget(glm::vec3(-1.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 5.f));
        auto currentTime = std::chrono::high_resolution_clock::now();



		while (!window.shouldClose())
		{
			glfwPollEvents();
			
            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            frameTime = glm::min(frameTime, MAX_FRAME_TIME);

            cameraController.moveInPlaneXZ(window.getGLFWwindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            float aspect = renderer.getAspectRatio();
            
            camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 1000.f);

			if (auto commandBuffer = renderer.beginFrame()) // begin the frame and get the command buffer
			{
				int frameIndex = renderer.getFrameIndex(); // get the current frame index

				FrameInfo frameInfo {
					frameIndex,
					frameTime,
					commandBuffer,
					camera,
					globalDescriptorSets[frameIndex],
					gameObjects
				};

				VTA_UI::FrameInfo_EditorUI editorUIFrameInfo
				{
					frameIndex,
					frameTime,
					commandBuffer,
					editorUIDescriptorSets[frameIndex],
					renderer.getScreenWidth(),
					renderer.getScreenHeight()
				};

				
				// update
				GlobalUbo ubo{};
				ubo.projectionMatrix = camera.getProjection();
				ubo.viewMatrix = camera.getView();
				ubo.inverseView = camera.getInverseView();
				pointLightSystemSystem.update(frameInfo, ubo); // update the point light system with the frame info and the uniform buffer object
				globalUboBuffer.writeToIndex(&ubo, frameIndex); // write the projection view matrix to the uniform buffer for the current frame
				globalUboBuffer.flushIndex(frameIndex); // flush the uniform buffer for the current frame

				// render game
				renderer.beginSwapChainRenderPass1(commandBuffer); // begin the render pass for the swap chain
				simpleRenderSystem.renderGameObjects(frameInfo); // render the game objects
				pointLightSystemSystem.render(frameInfo);
				renderer.endSwapChainRenderPass(commandBuffer); // end the render pass for the swap chain
				
				// render editor ui
				renderer.beginSwapChainRenderPass2(commandBuffer);
				editorUIRenderSystem.renderWidgets(editorUIFrameInfo);
				renderer.endSwapChainRenderPass(commandBuffer);
				
				renderer.endFrame(); // end the frame and submit the command buffer
				
			}
		}

		vkDeviceWaitIdle(device.device()); // wait for the device to finish all operations before destroying resources
	}

    

	void AppControl::loadWidgetObjects()
	{



		//std::shared_ptr<VTA_UI::VTAWidget> testWidget = VTA_UI::VTAWidget::create();
		//testWidget->model = testWidgetMesh;
		
		//testWidget->rectTransform.anchors = textAnchors;
		//testWidget->rectTransform.scale = { 0.5, 0.5, 0.5 };

		//widgets.push_back(std::move(testWidget));

		// below this line we create the box

		VTA_UI::Anchors boxAnchors;
		boxAnchors.min = { 0, 0.6 };
		boxAnchors.max = { 1, 1 };
		glm::vec2 boxPivots = { 0, 0 };
		VTA_UI::FlatMesh::Builder flatMeshBuilder2;
		flatMeshBuilder2.makeSimpleMesh(VTA_UI::FlatMesh::Rectangle, 1, 1, boxPivots, glm::vec4(0.5, 0.5, 0.5, 1));
		std::shared_ptr<VTA_UI::FlatMesh> boxWidgetMesh = std::make_shared<VTA_UI::FlatMesh>(device, flatMeshBuilder2);
		std::shared_ptr<VTA_UI::VTAWidget> boxWidget = VTA_UI::VTAWidget::create();
		boxWidget->model = boxWidgetMesh;
		boxWidget->rectTransform.anchors = boxAnchors;
		


		VTA_UI::Anchors innerBoxAnchors;
		innerBoxAnchors.min = { 0.5, 0.5 };
		innerBoxAnchors.max = { 1, 1 };
		VTA_UI::FlatMesh::Builder flatMeshBuilder3;
		flatMeshBuilder3.makeSimpleMesh(VTA_UI::FlatMesh::Rectangle, 1, 1, boxPivots, glm::vec4(0, 1, 0, 1));
		std::shared_ptr<VTA_UI::FlatMesh> innerWidgetMesh = std::make_shared<VTA_UI::FlatMesh>(device, flatMeshBuilder3);
		std::shared_ptr<VTA_UI::VTAWidget> innerWidget = VTA_UI::VTAWidget::create();
		innerWidget->model = innerWidgetMesh;
		innerWidget->rectTransform.anchors = innerBoxAnchors;
		


		VTA_UI::FlatMesh::Builder flatMeshBuilder;
		glm::vec2 pivots = { 0, 0 };
		flatMeshBuilder.makeTextMesh("Model1,   Model2", *fontAtlas, pivots, {0, 0, 0, 1});
		std::shared_ptr<VTA_UI::FlatMesh> textWidgetMesh = std::make_shared<VTA_UI::FlatMesh>(device, flatMeshBuilder);
		VTA_UI::Anchors textAnchors;
		textAnchors.min = { 0.5, 0.5 };
		textAnchors.max = { 0.5, 0.5 };
		textAnchors.size = { 1, 1}; // used as scale
		std::shared_ptr<VTA_UI::VTAWidget> textWidget = VTA_UI::VTAWidget::create();
		textWidget->model = textWidgetMesh;
		textWidget->rectTransform.anchors = textAnchors;
		textWidget->rectTransform.pivots = pivots;


		innerWidget->AddChild(std::move(textWidget));
		boxWidget->AddChild(std::move(innerWidget));
		widgets.push_back(std::move(boxWidget));
	}

	void AppControl::loadGameObjects()
	{
		std::shared_ptr<VTAModel> vaseModel1 = VTAModel::createModelFromFile(device, "../models/smooth_vase.obj"); // load the cube model from the file
		auto vase1 = VTAGameObject::createGameObject(); // create a game object
		vase1.model = vaseModel1; // set the model of the game object to the cube model
		vase1.transform.translation = { 0.f, 0.4f, 0.5f }; // set the translation of the game object
		vase1.transform.scale = { 0.5f, 0.5f, 0.5f }; // set the scale of the game object
		//cube.transform.rotation = { glm::radians(180.f), 0.f , 0.f };

		std::shared_ptr<VTAModel> vaseModel2 = VTAModel::createModelFromFile(device, "../models/smooth_vase.obj"); // load the cube model from the file
		auto vase2 = VTAGameObject::createGameObject(); // create a game object
		vase2.model = vaseModel2; // set the model of the game object to the cube model
		vase2.transform.translation = { 0.f, 0.4f, -0.5f }; // set the translation of the game object
		vase2.transform.scale = { 0.5f, 0.5f, 0.5f }; // set the scale of the game object
		//cube.transform.rotation = { glm::radians(180.f), 0.f , 0.f };

		std::shared_ptr<VTAModel> quadModel = VTAModel::createModelFromFile(device, "../models/quad.obj"); // load the cube model from the file
		auto quad = VTAGameObject::createGameObject(); // create a game object
		quad.model = quadModel; // set the model of the game object to the cube model
		quad.transform.translation = { 0.f, 0.5f, 0.f }; // set the translation of the game object
		quad.transform.scale = { 3.1f, 1.f, 3.f }; // set the scale of the game object

		gameObjects.emplace(vase2.getId(), std::move(vase2)); // add the game object to the vector of game objects
		gameObjects.emplace(vase1.getId(), std::move(vase1)); // add the game object to the vector of game objects
		gameObjects.emplace(quad.getId(), std::move(quad));
		

		std::vector<glm::vec3> lightColors{
		{1.f, 1.f, 1.f},
		 {1.f, .1f, .1f},
		 {.1f, .1f, 1.f},
		 {.1f, 1.f, .1f},
		 {1.f, 1.f, .1f},
		 {.1f, 1.f, 1.f},
		 {1.f, 1.f, 1.f}  };

		for (int i = 0; i < lightColors.size(); i++)
		{
			auto pointLight = VTAGameObject::makePointLight(0.3f);
			pointLight.color = lightColors[i];
			/*auto rotateLight = glm::rotate(
				glm::mat4(1.f),
				(i * glm::two_pi<float>() /2) / lightColors.size(),
				{ 0.f, -1.f, 0.f });
			pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-0.2f, -2.f, 0.f, 1.f));*/
			gameObjects.emplace(pointLight.getId(), std::move(pointLight)); // add the point light to the vector of game objects
		}
	}

	void AppControl::createFontAtlas()
	{
		fs::path fontPath = fs::current_path() / ".." / "fonts" / "Inter_default.ttf";

		// keep the string alive if you need a const char*
		std::string fontPathStr = fontPath.string();
		const char* filePath = fontPathStr.c_str();  // valid as long as fontPathStr lives
		float pixelHeight = 14;
		float firstCodePoint = 32;
		int codePointCount = 95;
		float atlasWidth = 256;
		float atlasHeight = 256;
		

		fontAtlas = new Trex::Atlas(fontPathStr.c_str(), 18, Trex::Charset::Ascii(), Trex::RenderMode::DEFAULT, 2);
	}

   

}




