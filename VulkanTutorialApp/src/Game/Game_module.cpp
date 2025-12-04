#include "Game_module.h"
#include <stdexcept>
#include <array>
#include <chrono>
#include <numeric>
#include "VTA_image.h"
#include "simple_render_system.h"
#include "VTA_swap_chain.hpp"

VTA::VTACamera VTA::Game_Module::camera = VTA::VTACamera();
VTA::VTAGameObject::Map* VTA::Game_Module::gameObjects = new VTA::VTAGameObject::Map();

VTA::Game_Module::Game_Module(VTA::ResourceLoader& loader, std::string rendererName, VTA::VTADevice& device, int framesInFlight) :
	device(device), viewerObject(VTAGameObject::createGameObject()), loader(loader)
{
	this->framesInFlight = framesInFlight;
	auto minOffsetAllignment = std::lcm(device.properties.limits.minUniformBufferOffsetAlignment, device.properties.limits.nonCoherentAtomSize);
	globalUboBuffer = new VTABuffer(device, sizeof(GlobalUbo), VTASwapChain::MAX_FRAMES_IN_FLIGHT,  // how many uniform buffer objects in our uniform buffer? one for each frame in flight
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, // double buffering like this should be used whenever we have a synamic resource that is written to each frame
		minOffsetAllignment);
	globalUboBuffer->map();

	Resource* uboResource = new Resource((void*)globalUboBuffer, ResourceType::Buffer);
	globalSet = SimpleRenderSystem::CreateGlobalResourceSet();
	globalSet->AddResource(uboResource);
	loader.LoadResourceSet(*globalSet);
	viewerObject.transform.translation = { 0.f, 0.f, 0.f};
	camera.setViewTarget(glm::vec3(-1.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 5.f));





	
	LoadResources();
	LoadModels();
	LoadGameObjects();
	LoadLights();



}

VTA::FrameInfo VTA::Game_Module::Update(
	GLFWwindow* window,
	float frameTime, float aspect, float frameIndex, glm::vec2 screenDims,
	VkCommandBuffer commandBuffer)
{
	FrameInfo frameInfo {
		frameIndex,
		frameTime,
		commandBuffer,
		camera,
		*gameObjects,
		*globalSet
	};


	GlobalUbo ubo{};
	cameraController.moveInPlaneXZ(window, frameTime, viewerObject);
	camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);
	camera.setPerspectiveProjection(glm::radians(50.f), aspect, 0.1f, 1000.f);
	ubo.projectionMatrix = camera.getProjection();
	ubo.viewMatrix = camera.getView();
	ubo.inverseView = camera.getInverseView();
	ubo.cameraPos = viewerObject.transform.translation;
	ubo.screenDims = screenDims;
	
	UpdateLights(frameInfo, ubo);
	globalUboBuffer->writeToIndex(&ubo, frameIndex);
	globalUboBuffer->flushIndex(frameIndex);


	return frameInfo;
}

VTA::VTAGameObject::Map& VTA::Game_Module::GetGameObjects()
{
	return *gameObjects;
}

VTA::ResourceSet VTA::Game_Module::GetGlobalResourceSet()
{
	return *globalSet;
}

glm::mat4 VTA::Game_Module::GetViewMatrix()
{
	return camera.getView();
}

glm::mat4 VTA::Game_Module::GetProjectionMatrix()
{
	return camera.getProjection();
}

void VTA::Game_Module::LoadResources()
{
	auto testTexture = new VTA_Image::Texture(device, "../Textures/OnyxTexture4K.jpg");
	auto testTextureResource = new Resource((void*) testTexture, ResourceType::Image);
	resources = {
		{"TestTexture", testTextureResource}
	};
}
void VTA::Game_Module::LoadModels()
{
	std::shared_ptr<VTAModel> vaseModel = VTAModel::createModelFromFile(device, "../models/smooth_vase.obj"); // load the cube model from the file
	std::shared_ptr<VTAModel> quadModel = VTAModel::createModelFromFile(device, "../models/quad.obj"); // load the cube model from the file
	std::shared_ptr<VTAModel> maleModel = VTAModel::createModelFromFile(device, "../models/Male.OBJ");
	std::shared_ptr<VTAModel> necromantModel = VTAModel::createModelFromFile(device, "../models/Necromant.obj");
	std::shared_ptr<VTAModel> duckModel = VTAModel::createModelFromFile(device, "../models/BountyHunter.obj");

	models =
	{
		{"Vase", vaseModel},
		{"Quad", quadModel},
		{"Male", maleModel},
		{"Necromant", necromantModel},
		{"Duck", duckModel}
	};
}
void VTA::Game_Module::LoadGameObjects()
{
	auto vase1 = VTAGameObject::createGameObject(); // create a game object
	vase1.model = models["Duck"]; // set the model of the game object to the cube model
	vase1.resources = SimpleRenderSystem::CreatePerObjectResourceSet();
	vase1.resources->AddResource(resources["TestTexture"]);
	loader.LoadResourceSet(*vase1.resources);
	vase1.transform.translation = { 0.f, 0.4f, 0.5f }; // set the translation of the game object
	vase1.transform.scale = { 0.1f, 0.1f, 0.1f }; // set the scale of the game object
	vase1.transform.rotation = { glm::radians(180.f), 0.f , 0.f };
	vase1.AddBoundingBox(glm::vec3(0.2, 0.4, 0.2f));
	vase1.name = "Human";

	auto vase2 = VTAGameObject::createGameObject(); // create a game object
	vase2.model = models["Vase"]; // set the model of the game object to the cube model
	vase2.resources = SimpleRenderSystem::CreatePerObjectResourceSet();
	vase2.resources->AddResource(resources["TestTexture"]);
	loader.LoadResourceSet(*vase2.resources);
	vase2.transform.translation = { 0.f, 0.f, 0.f }; // set the translation of the game object
	vase2.transform.scale = { 0.5f, 0.5f, 0.5f }; // set the scale of the game object
	//cube.transform.rotation = { glm::radians(180.f), 0.f , 0.f };
	vase2.AddBoundingBox(glm::vec3(0.2, 0.2, 0.2f));
	vase2.name = "Vase";


	auto quad = VTAGameObject::createGameObject(); // create a game object
	quad.model = models["Quad"]; // set the model of the game object to the cube model
	quad.resources = SimpleRenderSystem::CreatePerObjectResourceSet();
	quad.resources->AddResource(resources["TestTexture"]);
	loader.LoadResourceSet(*quad.resources);
	quad.transform.translation = { 0.f, 0.5f, 0.f }; // set the translation of the game object
	quad.transform.scale = { 3.1f, 1.f, 3.f }; // set the scale of the game object
	quad.AddBoundingBox(glm::vec3(4, 0.05, 4));
	quad.name = "Surface";

	gameObjects->emplace(vase2.getId(), std::move(vase2)); // add the game object to the vector of game objects
	gameObjects->emplace(vase1.getId(), std::move(vase1)); // add the game object to the vector of game objects
	gameObjects->emplace(quad.getId(), std::move(quad));
}

void VTA::Game_Module::LoadLights()
{
		std::vector<glm::vec3> lightColors{
	{1.f, 1.f, 1.f},
	 {1.f, 1.f, 1.f},
	 {1.f, 1.f, 1.f},
	 {1.f, 1.f, 1.f},
	{1.f, 1.f, 1.f},
	 {1.f, 1.f, 1.f},
	 {1.f, 1.f, 1.f} };

		for (int i = 0; i < lightColors.size(); i++)
		{
			auto pointLight = VTAGameObject::makePointLight(0.3f);
			pointLight.color = lightColors[i];
			/*auto rotateLight = glm::rotate(
				glm::mat4(1.f),
				(i * glm::two_pi<float>() /2) / lightColors.size(),
				{ 0.f, -1.f, 0.f });
			pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-0.2f, -2.f, 0.f, 1.f));*/
			pointLight.transform.translation = glm::vec3(0, 0, i * 0.5);
			pointLight.AddBoundingBox(glm::vec3(0.1, 0.1, 0.1));
			pointLight.name = "Point light";
			gameObjects->emplace(pointLight.getId(), std::move(pointLight)); // add the point light to the vector of game objects
			
		}
}

void VTA::Game_Module::UpdateLights(FrameInfo frameInfo, GlobalUbo& ubo)
{
	int lightIndex = 0;
	for (auto& kv : frameInfo.gameObjects)
	{
		auto& obj = kv.second;
		if (obj.pointLight == nullptr) continue;

		ubo.pointLightS[lightIndex].position = glm::vec4(obj.transform.translation, 1.f);
		ubo.pointLightS[lightIndex].color = glm::vec4(obj.color, obj.pointLight->lightIntensity);
		lightIndex++;
	}

	ubo.numLights = lightIndex;
}



// this should later be moved to a game/scene renderer class
void VTA::Game_Module::BeginFrame(uint32_t frameIndex)
{
	currentCommandBuffer = commandBuffers[frameIndex];
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	if (vkBeginCommandBuffer(currentCommandBuffer, &beginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to begin recording command buffer!");
	}

}


void VTA::Game_Module::EndFrame()
{
}
