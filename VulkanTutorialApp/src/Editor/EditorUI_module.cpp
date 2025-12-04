#include "EditorUI_module.h"
#include <filesystem>
#include <stdexcept>
#include "VTA_swap_chain.hpp"
#include "Game_module.h"
#include "stb_truetype.h"
#include "VTA_image.h"
namespace fs = std::filesystem;

namespace VTA_UI
{
	VTA_UI::EditorUI_Module::EditorUI_Module(
		VTA::ResourceLoader& loader,
		std::string renderingSystemName,
		VTA::VTADevice& device,
		VTA::VTAWindow& window)
		: loader(loader), device(device), renderingSystemName(renderingSystemName), window(window)
	{
		LoadResources();
		LoadStartUpWidgets();
		uiController = std::make_unique<UIController>();
		inspectionWindowEnabled = false;
	}

	void EditorUI_Module::Update()
	{
		uiController->SetWindow(window.getGLFWwindow());
		uiController->SelectObject(
			VTA::Game_Module::GetViewMatrix(),
			VTA::Game_Module::GetProjectionMatrix());

		int selectedId;
		if (uiController->GetSelectedObject(selectedId))
		{
			DisplayObjectInformation(selectedId);
			glm::vec2 clickPos;
			if (uiController->GetClickPos(clickPos))
			{
				if (testClickable->CheckClickOverlap(clickPos))
				{
					testClickable->Clicked();
				}

			}
		}
		else if (inspectionWindowEnabled)
		{
			CloseInspectionWindow();
		}



	}

	void VTA_UI::EditorUI_Module::LoadStartUpWidgets()
	{
		/* VTA_UI::Anchors boxAnchors;
		boxAnchors.min = { 0, 0.6 };
		boxAnchors.max = { 1, 1 };
		glm::vec2 boxPivots = { 0, 0 };
		auto box = CreateShapeWidget(FlatMesh::Rectangle, boxAnchors, boxPivots, "", { 0.1, 1, 1, 1 });

		VTA_UI::Anchors innerBoxAnchors;
		innerBoxAnchors.min = { 0.5, 0.5 };
		innerBoxAnchors.max = { 1, 1 };
		glm::vec2 innerBoxPivots = { 0, 0 };
		auto innerBox = CreateShapeWidget(FlatMesh::Rectangle, innerBoxAnchors, innerBoxPivots, "", { 1, 0.1, 1, 1 });

		VTA_UI::Anchors textAnchors;
		textAnchors.min = { 0.5, 0.5 };
		textAnchors.max = { 0.5, 0.5 };
		textAnchors.size = { 1, 1 }; // used as scale
		glm::vec2 textPivots = { 0, 0 };
		auto text = CreateTextWidget(textAnchors, textPivots, "18pt_FontBitMap", { 0, 0, 0, 1 });

		innerBox->AddChild(text);
		box->AddChild(innerBox); */
	}

	void VTA_UI::EditorUI_Module::LoadResources()
	{
		fs::path fontPath1 = fs::current_path() / ".." / "fonts" / "Inter_Variable.ttf";


		/*fs::path fontPath2 = fs::current_path() / ".." / "fonts" / "Inter_18pt-Bold.ttf";
		auto fontAtlas2 = new Trex::Atlas(fontPath2.string().c_str(), 18, Trex::Charset::Ascii(), Trex::RenderMode::DEFAULT, 2);
		VTA_Image::Texture* fontAtlasTexture2 = new VTA_Image::Texture(device, *fontAtlas2);*/

		fontPathRefs = std::unordered_map<std::string, std::string>
		{
			{"Inter", fontPath1.string().c_str()},
			/*{"18ptBold_FontBitmap", fontAtlas2}*/
		};
	}

	VTA::Resource* EditorUI_Module::FindOrLoadFontTexture(std::string fontName, int size)
	{
		auto pathRef = fontPathRefs.find(fontName);

		if (pathRef == fontPathRefs.end())
		{
			throw std::runtime_error("not a valid font path");
		}

		auto path = pathRef->second;
		auto resourceName = fontName + std::to_string(size);
		if (ownedResources.find(resourceName) == ownedResources.end())
		{
			// we need to create a font atlas and the corresponding resource
			auto atlas = std::make_shared<Trex::Atlas>(path, size, Trex::Charset::Ascii(), Trex::RenderMode::DEFAULT, 2);
			VTA_Image::Texture* fontAtlasTexture = new VTA_Image::Texture(device, *atlas);

			fontAtlasRefs[resourceName] = atlas;
			ownedResources[resourceName] = new VTA::Resource(fontAtlasTexture, VTA::ResourceType::Image);
		}

		return ownedResources[resourceName];

	}

	std::shared_ptr<Trex::Atlas> EditorUI_Module::FindFontAtlas(std::string fontName, int size)
	{
		auto pathRef = fontPathRefs.find(fontName);

		if (pathRef == fontPathRefs.end())
		{
			throw std::runtime_error("not a valid font path");
		}

		auto path = pathRef->second;
		auto resourceName = fontName + std::to_string(size);

		return fontAtlasRefs[resourceName];


	}

	void EditorUI_Module::CreateInspectorPanel()
	{

	}

	void EditorUI_Module::CreateSceneHierarchyPanel()
	{

	}



}



