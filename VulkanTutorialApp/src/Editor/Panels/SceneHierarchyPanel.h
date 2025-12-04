#pragma once
#include "Game/Scene.h"
#include "Game/VTA_game_object.h"

namespace VTA_UI
{
    class SceneHierarchyPanel 
    {
        void OnImGuiRender(bool& isOpen);
        void DrawGameObjectNode(VTA::VTAGameObject);

    private:
        VTA::Scene* context;
    };
}

