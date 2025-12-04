#pragma once
#include "VTA_device.hpp"
#include "Core/Module.h"

namespace VTA
{
	class ImGuiRenderSystem 
	{
	public:
		ImGuiRenderSystem();
		ImGuiRenderSystem(const std::string& name);
		virtual ~ImGuiRenderSystem();

		virtual void BeginUIFrame();
		virtual void EndUIFrame();

		virtual void OnAttach();
		virtual void OnDetach();
	private:
		void SetDarkThemeV2Colors();
		VkCommandBuffer m_CommandBuffer = VK_NULL_HANDLE;
		float m_Time = 0.0f;
	};
}




