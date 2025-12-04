#include "ImGuiModule.h"
#include "imgui.h"
#include "ImGui/ImGuiFonts.h"
#include "VTA_renderer.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"




static void check_vk_result(VkResult err)
{
	if (err == VK_SUCCESS)
		return;
	fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
	if (err < 0)
		abort();
}




namespace VTA
{
	static std::vector<VkCommandBuffer> s_ImGuiCommandBuffers;

	ImGuiRenderSystem::ImGuiRenderSystem()
	{
	}

	ImGuiRenderSystem::ImGuiRenderSystem(const std::string& name)
	{
	}

	ImGuiRenderSystem::~ImGuiRenderSystem()
	{
	}

	void ImGuiRenderSystem::OnAttach()
	{
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		//io.ConfigFlags |= ImGuiConfigFlags_Na;           // Enable Docking
		//io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows


		// configure fonts
		{
			VTA_UI::FontConfiguration inter;
			inter.FontName = "Inter";
			inter.FilePath = "Fonts/Inter_Variable";
			inter.Size = 18.0f;
			VTA_UI::Fonts::Add(inter);
		}

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		SetDarkThemeV2Colors();

		// the renderer part of the configuration
		GLFWwindow* window = VTAWindow::instance->getGLFWwindow();
		auto device = VTADevice::singleton->device();
		VkDescriptorPool descriptorPool;

		VkDescriptorPoolSize pool_sizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 100 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 100 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 100 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 100 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 100 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 100 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 100 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 100 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 100 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 100 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 100 }
		};

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		pool_info.maxSets = 100 * IM_ARRAYSIZE(pool_sizes);
		pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
		pool_info.pPoolSizes = pool_sizes;

		if (vkCreateDescriptorPool(device, &pool_info, nullptr, &descriptorPool) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor pool!");
		}

		// Setup Platform/Renderer bindings
		ImGui_ImplGlfw_InitForVulkan(window, true);
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = VTADevice::singleton->getInstance();
		init_info.PhysicalDevice = VTADevice::singleton->getPhysicalDevice();
		init_info.Device = device;
		init_info.QueueFamily = VTADevice::singleton->findPhysicalQueueFamilies().graphicsFamily;
		init_info.Queue = VTADevice::singleton->graphicsQueue();
		init_info.PipelineCache = nullptr;
		init_info.DescriptorPool = descriptorPool;
		init_info.Allocator = nullptr;
		init_info.MinImageCount = 2;
		init_info.ImageCount = VTARenderer::getSwapchainImageCount();
		

		init_info.PipelineInfoMain.RenderPass = VTARenderer::getGUIRenderPass();
		init_info.PipelineInfoMain.Subpass = 0;
		init_info.PipelineInfoMain.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		init_info.CheckVkResultFn = check_vk_result;
		ImGui_ImplVulkan_Init(&init_info);

		// now lets save references to the command buffers that we will use when rendering
		uint32_t framesInFlight = VTARenderer::getFramesInFlight();
		s_ImGuiCommandBuffers.resize(framesInFlight);
		for (uint32_t i = 0; i < framesInFlight; i++)
		{
			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; // primary command buffers can be submitted to a queue, secondary command buffers can be called from primary command buffers but cannot be submitted to a queue
			allocInfo.commandPool = VTADevice::singleton->getCommandPool(); // command pools are pooling. I think this is a memory optimization
			allocInfo.commandBufferCount = 1;

			if (vkAllocateCommandBuffers(VTADevice::singleton->device(), &allocInfo, &s_ImGuiCommandBuffers[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to allocate command buffers!");
			}
		}



	}


	void ImGuiRenderSystem::BeginUIFrame()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void ImGuiRenderSystem::EndUIFrame()
	{
		ImGui::Render();
		
		VkClearValue clearValues[2];
		clearValues[0].color = { {0.1f, 0.1f,0.1f, 1.0f} };
		clearValues[1].depthStencil = { 1.0f, 0 };

		uint32_t width = VTARenderer::getScreenWidth();
		uint32_t height = VTARenderer::getScreenHeight();

		VkCommandBuffer guiCommandBuffer = s_ImGuiCommandBuffers[VTARenderer::getFrameIndex()];
		
		VkCommandBufferBeginInfo drawCmdBufInfo = {};
		drawCmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		drawCmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		drawCmdBufInfo.pNext = nullptr;


		VkCommandBuffer drawCommandBuffer = VTARenderer::getCurrentEditorUICommandBuffer();
		if (vkBeginCommandBuffer(drawCommandBuffer, &drawCmdBufInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to begin the ui command buffer");
		}


		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.pNext = nullptr;
		renderPassBeginInfo.renderPass = VTARenderer::getGUIRenderPass();
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent.width = width;
		renderPassBeginInfo.renderArea.extent.height = height;
		renderPassBeginInfo.clearValueCount = 2; // Color + depth
		renderPassBeginInfo.pClearValues = clearValues;
		renderPassBeginInfo.framebuffer = VTARenderer::getCurrentFrameBufferPass2();

		vkCmdBeginRenderPass(drawCommandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);


		VkCommandBufferInheritanceInfo inheritanceInfo = {};
		inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inheritanceInfo.renderPass = VTARenderer::getGUIRenderPass();
		inheritanceInfo.framebuffer = VTARenderer::getCurrentFrameBufferPass2();

		VkCommandBufferBeginInfo cmdBufInfo = {};
		cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdBufInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
		cmdBufInfo.pInheritanceInfo = &inheritanceInfo;

		if (vkBeginCommandBuffer(guiCommandBuffer, &cmdBufInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to begin the ui command buffer");
		}

		VkViewport viewport = {};
		viewport.x = 0.0f;
		viewport.y = (float)height;
		viewport.height = -(float)height;
		viewport.width = (float)width;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(guiCommandBuffer, 0, 1, &viewport);

		VkRect2D scissor = {};
		scissor.extent.width = width;
		scissor.extent.height = height;
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		vkCmdSetScissor(guiCommandBuffer, 0, 1, &scissor);

		ImDrawData* main_draw_data = ImGui::GetDrawData();
		ImGui_ImplVulkan_RenderDrawData(main_draw_data, guiCommandBuffer);

		if (vkEndCommandBuffer(guiCommandBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record ui command buffer");
		}

		std::vector<VkCommandBuffer> commandBuffers;
		commandBuffers.push_back(guiCommandBuffer);

		vkCmdExecuteCommands(drawCommandBuffer, uint32_t(commandBuffers.size()), commandBuffers.data());
		vkCmdEndRenderPass(drawCommandBuffer);

		if (vkEndCommandBuffer(drawCommandBuffer) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to record draw command buffer");
		}
	}


	void ImGuiRenderSystem::OnDetach()
	{
		auto device = VTADevice::singleton->device();
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void ImGuiRenderSystem::SetDarkThemeV2Colors()
	{
		auto& style = ImGui::GetStyle();
		auto& colors = ImGui::GetStyle().Colors;

		//========================================================
		/// Colours

		// Headers
		colors[ImGuiCol_Header] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::groupHeader);
		colors[ImGuiCol_HeaderHovered] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::groupHeader);
		colors[ImGuiCol_HeaderActive] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::groupHeader);

		// Buttons
		colors[ImGuiCol_Button] = ImColor(56, 56, 56, 200);
		colors[ImGuiCol_ButtonHovered] = ImColor(70, 70, 70, 255);
		colors[ImGuiCol_ButtonActive] = ImColor(56, 56, 56, 150);

		// Frame BG
		colors[ImGuiCol_FrameBg] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::propertyField);
		colors[ImGuiCol_FrameBgHovered] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::propertyField);
		colors[ImGuiCol_FrameBgActive] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::propertyField);

		// Tabs
		colors[ImGuiCol_Tab] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::titlebar);
		colors[ImGuiCol_TabHovered] = ImColor(255, 225, 135, 30);
		colors[ImGuiCol_TabActive] = ImColor(255, 225, 135, 60);
		colors[ImGuiCol_TabUnfocused] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::titlebar);
		colors[ImGuiCol_TabUnfocusedActive] = colors[ImGuiCol_TabHovered];

		// Title
		colors[ImGuiCol_TitleBg] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::titlebar);
		colors[ImGuiCol_TitleBgActive] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::titlebar);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Resize Grip
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.91f, 0.91f, 0.91f, 0.25f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.81f, 0.81f, 0.81f, 0.67f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.46f, 0.46f, 0.46f, 0.95f);

		// Scrollbar
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.0f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.0f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.0f);

		// Check Mark
		colors[ImGuiCol_CheckMark] = ImColor(200, 200, 200, 255);

		// Slider
		colors[ImGuiCol_SliderGrab] = ImVec4(0.51f, 0.51f, 0.51f, 0.7f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.66f, 0.66f, 0.66f, 1.0f);

		// Text
		colors[ImGuiCol_Text] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::text);

		// Checkbox
		colors[ImGuiCol_CheckMark] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::text);

		// Separator
		colors[ImGuiCol_Separator] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::backgroundDark);
		colors[ImGuiCol_SeparatorActive] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::highlight);
		colors[ImGuiCol_SeparatorHovered] = ImColor(39, 185, 242, 150);

		// Window Background
		colors[ImGuiCol_WindowBg] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::titlebar);
		colors[ImGuiCol_ChildBg] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::background);
		colors[ImGuiCol_PopupBg] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::backgroundPopup);
		colors[ImGuiCol_Border] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::backgroundDark);

		// Tables
		colors[ImGuiCol_TableHeaderBg] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::groupHeader);
		colors[ImGuiCol_TableBorderLight] = ImGui::ColorConvertU32ToFloat4(Colors::Theme::backgroundDark);

		// Menubar
		colors[ImGuiCol_MenuBarBg] = ImVec4{ 0.0f, 0.0f, 0.0f, 0.0f };

		//========================================================
		/// Style
		style.FrameRounding = 2.5f;
		style.FrameBorderSize = 1.0f;
		style.IndentSpacing = 11.0f;
	}

}