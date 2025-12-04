#pragma once
#include "VTA_game_object.h"
#include "Core/UUID.h"

namespace VTA
{
    class Scene
    {
        public:
            VTAGameObject* GetObjectWithUUID(UUID uuid);
    };
}