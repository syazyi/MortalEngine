#include "ui.h"

namespace mortal
{
    UI::UI(RenderingSystemInfo& info) : RenderPartBase(info)
    {
        Init();
    }

    UI::~UI()
    {
        ClearUp();
    }

    void UI::Init()
    {
        //Create Imgui DescriptorPool
        auto& device = m_RenderingInfo.device.GetDevice();
        auto& swapchain = m_RenderingInfo.swapchain;
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
        m_DescriptorPool = device.createDescriptorPool(poolCreateInfo);

        vk::AttachmentDescription attach_Present({}, swapchain.GetSurfaceDetail().SurfaceFormats.format, vk::SampleCountFlagBits::e1,
            vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
            vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR);
        vk::AttachmentReference colorRef(0, vk::ImageLayout::eColorAttachmentOptimal);
        vk::SubpassDescription subPassDes({}, vk::PipelineBindPoint::eGraphics, {}, colorRef, {}, nullptr);
        vk::SubpassDependency subPassDep(VK_SUBPASS_EXTERNAL, 0, vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eColorAttachmentOutput, 
            vk::AccessFlagBits::eNone, vk::AccessFlagBits::eColorAttachmentWrite);
        vk::RenderPassCreateInfo passCreateInfo({}, attach_Present, subPassDes, subPassDep);
        m_UIPass = device.createRenderPass(passCreateInfo);

        auto extent2D = m_RenderingInfo.window.GetExtent2D();
        for (auto& imageView : swapchain.GetSwapChainImageViews()) {
            std::array<vk::ImageView, 1> attachments{ imageView };
            vk::FramebufferCreateInfo framebufferCI({}, m_UIPass, attachments, extent2D.width, extent2D.height, 1);
            m_FrameBuffers.push_back(device.createFramebuffer(framebufferCI));
        }

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
        init_info.PipelineCache = m_PipelineCache;
        init_info.DescriptorPool = m_DescriptorPool;
        init_info.Subpass = 0;
        init_info.MinImageCount = 2;
        init_info.ImageCount = m_RenderingInfo.swapchain.GetSwapChainImageViews().size();
        init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
        init_info.Allocator = nullptr;
        init_info.CheckVkResultFn = nullptr;
        ImGui_ImplVulkan_Init(&init_info, m_UIPass);

        //Upload Fonts
        auto cmd = m_RenderingInfo.command.BeginSingleCommand();
        ImGui_ImplVulkan_CreateFontsTexture(cmd);
        m_RenderingInfo.command.EndSingleCommand(cmd, m_RenderingInfo.device.GetRenderingQueue().GraphicQueue.value());

        device.waitIdle();
        ImGui_ImplVulkan_DestroyFontUploadObjects();
    }

    void UI::ClearUp()
    {
        auto& device = m_RenderingInfo.device.GetDevice();
        device.waitIdle();
        ImGui_ImplVulkan_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        device.destroyDescriptorPool(m_DescriptorPool);
        for (auto& framebuffer : m_FrameBuffers) {
            device.destroyFramebuffer(framebuffer);
        }
        device.destroyRenderPass(m_UIPass);
    }

    void UI::Draw()
    {
        auto& drawCmd = m_RenderingInfo.command.GetCommandBuffers()[m_RenderingInfo.CurrentFrame];
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::ShowDemoWindow(&show_demo_window);

        ImGui::Render();
        ImDrawData* draw_data = ImGui::GetDrawData();
        const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
        if (!is_minimized)
        {
            vk::ClearColorValue clearValue(std::array<float, 4>{1.0f, 1.0f , 1.0f, 1.0f });
            vk::ClearValue color(clearValue);
            vk::ClearValue depth(vk::ClearDepthStencilValue(1.0f, 0));
            std::array<vk::ClearValue, 2> clearValues{ color, depth};
            vk::Rect2D rect2D({}, m_RenderingInfo.window.GetExtent2D());
            drawCmd.begin(vk::CommandBufferBeginInfo());
            vk::RenderPassBeginInfo uiPassBegin(m_UIPass, m_FrameBuffers[m_RenderingInfo.nextImageIndex], rect2D, clearValues);
            drawCmd.beginRenderPass(uiPassBegin, vk::SubpassContents::eInline);
            ImGui_ImplVulkan_RenderDrawData(draw_data, drawCmd);
            drawCmd.endRenderPass();
            drawCmd.end();
        }
    }

} // namespace mortal
