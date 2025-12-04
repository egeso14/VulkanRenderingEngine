#include "Resource.h"
#include <unordered_map>
#include "VTA_device.hpp"
#include "Trex/Atlas.hpp"
#include "UIController.h"
#include <string>


namespace VTA_UI
{
	class EditorUI_Module
	{
	public:

		EditorUI_Module(VTA::ResourceLoader& loader, std::string rendererName, VTA::VTADevice& device, VTA::VTAWindow& window);
		void Update();

	private:
		VTA::ResourceLoader& loader;
		VTA::VTAWindow& window;
		std::string renderingSystemName;
		std::unique_ptr<UIController>  uiController;
		void LoadStartUpWidgets();
		std::unordered_map<std::string, VTA::Resource*> ownedResources;
		std::unordered_map<std::string, std::shared_ptr<Trex::Atlas>> fontAtlasRefs;
		std::unordered_map<std::string, std::string> fontPathRefs;
		void LoadResources();
		VTA::Resource* FindOrLoadFontTexture(std::string fontName, int size);
		std::shared_ptr<Trex::Atlas> FindFontAtlas(std::string fontName, int size);
		void CreateInspectorPanel();
		void CreateSceneHierarchyPanel();
		void DisplayObjectInformation(int objectId);
		
		VTA::VTADevice& device;
		

		bool inspectionWindowEnabled;
	};
}