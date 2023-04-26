#include "uiTool.h"

namespace mortal
{
    void mortal::UITool::InitUI(vk::RenderPass render_pass)
    {
        //Test UI
        std::vector<vk::DescriptorPoolSize> poolsizes{
            { vk::DescriptorType::eSampler, 1000 },
            { vk::DescriptorType::eCombinedImageSampler, 1000 },
            { vk::DescriptorType::eSampledImage, 1000 },
            { vk::DescriptorType::eStorageImage, 1000 },
            { vk::DescriptorType::eUniformTexelBuffer, 1000 },
            { vk::DescriptorType::eStorageTexelBuffer, 1000 },
            { vk::DescriptorType::eUniformBuffer, 1000 },
            { vk::DescriptorType::eStorageBuffer, 1000 },
            { vk::DescriptorType::eUniformBufferDynamic, 1000 },
            { vk::DescriptorType::eStorageBufferDynamic, 1000 },
            { vk::DescriptorType::eInputAttachment, 1000 }
        };
        vk::DescriptorPoolCreateInfo poolCreateInfo({}, 1000 * poolsizes.size(), poolsizes);
        m_UIPool = m_RenderingInfo.device.GetDevice().createDescriptorPool(poolCreateInfo);

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsLight();

        // Setup Platform/Renderer backends
        ImGui_ImplGlfw_InitForVulkan(m_RenderingInfo.window.GetWindow(), true);
        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = m_RenderingInfo.device.GetInstanceRef();
        init_info.PhysicalDevice = m_RenderingInfo.device.GetPhysicalDevice();
        init_info.Device = m_RenderingInfo.device.GetDevice();
        init_info.QueueFamily = m_RenderingInfo.device.GetRenderingQueue().GraphicQueueFamilyIndex.value();
        init_info.Queue = m_RenderingInfo.device.GetRenderingQueue().GraphicQueue.value();
        init_info.PipelineCache = nullptr;
        init_info.DescriptorPool = m_UIPool;
        init_info.Subpass = 0;
        init_info.MinImageCount = 2;
        init_info.ImageCount = m_RenderingInfo.swapchain.GetSwapChainImageViews().size();
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        init_info.Allocator = nullptr;
        init_info.CheckVkResultFn = nullptr;
        ImGui_ImplVulkan_Init(&init_info, render_pass);

        //Upload Fonts
        auto cmd = m_RenderingInfo.command.BeginSingleCommand();
        ImGui_ImplVulkan_CreateFontsTexture(cmd);
        m_RenderingInfo.command.EndSingleCommand(cmd, m_RenderingInfo.device.GetRenderingQueue().GraphicQueue.value());

        m_RenderingInfo.device.GetDevice().waitIdle();
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }

    void UITool::Draw(vk::CommandBuffer drawCmd, std::function<void(void)> uiFunc)
    {
        //Test
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        uiFunc();

        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();
        const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
        //Test
        if (!is_minimized)
        {
            ImGui_ImplVulkan_RenderDrawData(draw_data, drawCmd);
        }
    }

    void UITool::ClearUpUI()
    {
        //Test
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        m_RenderingInfo.device.GetDevice().destroyDescriptorPool(m_UIPool);
        //Test
    }
} // namespace mortal
