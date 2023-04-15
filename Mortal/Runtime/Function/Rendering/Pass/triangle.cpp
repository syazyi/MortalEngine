#include "triangle.h"

namespace mortal
{
    TrianglePass::TrianglePass(RenderingSystemInfo& info) : RenderPassBase(info)
    {
        Init();
    }
    TrianglePass::~TrianglePass()
    {
        ClearUp();
    }

    void TrianglePass::Init()
    {
        auto& device = m_RenderingInfo.device.GetDevice();
        //create pipeline layout
        {
            vk::PipelineLayoutCreateInfo createInfo({});
            m_PipelineLayout = device.createPipelineLayout(createInfo);
        }

        //set RenderPass
        {
            //About framebuffer description
            auto format = m_RenderingInfo.swapchain.GetSurfaceDetail().SurfaceFormats.format;
            vk::AttachmentDescription attachDes({}, format, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, 
                vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR);
            std::array<vk::AttachmentDescription, 1> attachDescriptions{ attachDes };

            vk::AttachmentReference colorAttachRef(0, vk::ImageLayout::eColorAttachmentOptimal);
            vk::SubpassDescription subPassDes({}, vk::PipelineBindPoint::eGraphics, {}, colorAttachRef);

            vk::SubpassDependency dependency(VK_SUBPASS_EXTERNAL, 0, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput,
                vk::AccessFlagBits::eNone, vk::AccessFlagBits::eColorAttachmentWrite);

            vk::RenderPassCreateInfo renderPassCreateInfo({}, attachDescriptions, subPassDes, dependency);
            m_RenderPass = device.createRenderPass(renderPassCreateInfo);
        }

        auto extent2d = m_RenderingInfo.window.GetExtent2D();
        //Set Framebuffer
        {
            auto& imageViews = m_RenderingInfo.swapchain.GetSwapChainImageViews();
            auto size = imageViews.size();
            m_FrameBuffers.resize(size);
            for (size_t i = 0; i < size; i++) {
                std::vector<vk::ImageView> imageViewNeeds{ 
                    imageViews[i]
                };
                vk::FramebufferCreateInfo framebufferCreateInfo({}, m_RenderPass, imageViewNeeds, extent2d.width, extent2d.height, 1);
                m_FrameBuffers[i] = device.createFramebuffer(framebufferCreateInfo);
            }
        }

        //set shader module
        {
            auto bCodeVert = LoadShader("triangle/triangle_vert");
            vk::ShaderModuleCreateInfo vertCreateInfo({}, bCodeVert.size(), reinterpret_cast<uint32_t*>(bCodeVert.data()));
            m_VertexShaderModule = device.createShaderModule(vertCreateInfo);

            auto bCodeFrag = LoadShader("triangle/triangle_frag");
            vk::ShaderModuleCreateInfo fragCreateInfo({}, bCodeFrag.size(), reinterpret_cast<uint32_t*>(bCodeFrag.data()));
            m_FragmentShaderModule = device.createShaderModule(fragCreateInfo);
        }

        //create pipeline
        {
            //shader stage
            vk::PipelineShaderStageCreateInfo vertStageCreateInfo({}, vk::ShaderStageFlagBits::eVertex, m_VertexShaderModule, "main");
            vk::PipelineShaderStageCreateInfo fragStageCreateInfo({}, vk::ShaderStageFlagBits::eFragment, m_FragmentShaderModule, "main");

            std::array<vk::PipelineShaderStageCreateInfo, 2> StageCreateInfos{ vertStageCreateInfo, fragStageCreateInfo };

            //vertex input state
            //vk::VertexInputBindingDescription bindDes();
            //vk::VertexInputAttributeDescription attrDes();

            vk::PipelineVertexInputStateCreateInfo vertInputStateInfo({}, {}, {});

            //input assembly state
            vk::PipelineInputAssemblyStateCreateInfo inputAssStateInfo({}, vk::PrimitiveTopology::eTriangleList, VK_FALSE);

            //viewport state
            
            vk::Viewport viewport(0, 0, extent2d.width, extent2d.height, 0.f, 1.f);
            vk::Rect2D scissor({}, extent2d);
            vk::PipelineViewportStateCreateInfo viewportStateInfo({}, viewport, scissor);

            //rasterization state
            vk::PipelineRasterizationStateCreateInfo rasterStateInfo({}, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eClockwise, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f);

            //multisample state
            vk::PipelineMultisampleStateCreateInfo multiSampleStateInfo({}, vk::SampleCountFlagBits::e1, VK_FALSE);

            //depth stencil state
            vk::PipelineDepthStencilStateCreateInfo depthStencilStateInfo({}, VK_FALSE, VK_FALSE);

            //color blend state
            vk::PipelineColorBlendAttachmentState colorBlendAttaState(VK_FALSE, vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eOneMinusSrcAlpha, vk::BlendOp::eAdd, 
                vk::BlendFactor::eZero, vk::BlendFactor::eOne, vk::BlendOp::eAdd, 
                vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
            vk::PipelineColorBlendStateCreateInfo colorBlendStateInfo({}, VK_FALSE, vk::LogicOp::eCopy, colorBlendAttaState);

            //dynamic state 
            std::array<vk::DynamicState, 2> dynamicStates{ vk::DynamicState::eViewport, vk::DynamicState::eScissor };
            vk::PipelineDynamicStateCreateInfo dynamicStateInfo({}, dynamicStates);

            vk::GraphicsPipelineCreateInfo pipelineCreateInfo({}, StageCreateInfos, &vertInputStateInfo, &inputAssStateInfo, nullptr, &viewportStateInfo, 
                &rasterStateInfo, &multiSampleStateInfo, &depthStencilStateInfo, &colorBlendStateInfo, &dynamicStateInfo, m_PipelineLayout, m_RenderPass, 0, nullptr, -1);
            auto result_pipeline = device.createGraphicsPipeline(nullptr, pipelineCreateInfo);
            m_Pipeline = result_pipeline.value;
        }

        //create semaphore and fence
        {
            vk::SemaphoreCreateInfo createInfo{};
            vk::FenceCreateInfo fCreateInfo(vk::FenceCreateFlagBits::eSignaled);
            for (uint32_t i = 0; i < MaxFrameInFlight; i++) {
                m_GetImageSemaphores[i] = device.createSemaphore(createInfo);
                m_PresentSemaphores[i] = device.createSemaphore(createInfo);
                m_FrameFences[i] = device.createFence(fCreateInfo);
            }
        }

        m_DrawCmds = m_RenderingInfo.command.GetCommandBuffers();
    }

    void TrianglePass::ClearUp()
    {
        auto& device = m_RenderingInfo.device.GetDevice();
        device.waitIdle();
        for (uint32_t i = 0; i < MaxFrameInFlight; i++) {
            device.destroyFence(m_FrameFences[i]);
            device.destroySemaphore(m_PresentSemaphores[i]);
            device.destroySemaphore(m_GetImageSemaphores[i]);
        }

        device.destroyPipeline(m_Pipeline);
        device.destroyShaderModule(m_FragmentShaderModule);
        device.destroyShaderModule(m_VertexShaderModule);
        for (auto& framebuffer : m_FrameBuffers) {
            device.destroyFramebuffer(framebuffer);
        }
        device.destroyRenderPass(m_RenderPass);
        device.destroyPipelineLayout(m_PipelineLayout);
    }

    void mortal::TrianglePass::Draw()
    {
        auto& device = m_RenderingInfo.device.GetDevice();
        auto& currentFrame = m_RenderingInfo.CurrentFrame;
        auto result_waitFence = device.waitForFences(m_FrameFences[currentFrame],VK_TRUE, UINT64_MAX);

        auto& swapchain = m_RenderingInfo.swapchain.GetSwapChain();
        auto result_nextImageIndex = device.acquireNextImageKHR(swapchain, UINT64_MAX, m_GetImageSemaphores[currentFrame]);
        auto nextImageIndex = result_nextImageIndex.value;

        device.resetFences(m_FrameFences[currentFrame]);

        auto& drawCmd = m_DrawCmds[currentFrame];
        drawCmd.reset();
        //Record Draw Cmd
        {
            vk::CommandBufferBeginInfo beginInfo{};
            drawCmd.begin(beginInfo);

            vk::Rect2D rect2d({}, m_RenderingInfo.window.GetExtent2D());
            vk::ClearValue clearValue;
            clearValue.setDepthStencil(vk::ClearDepthStencilValue(1.0f, 0));
            std::array<float, 4> clearColorValueBlack{ 0.0f, 0.0f, 0.0f, 1.0f };
            clearValue.setColor(vk::ClearColorValue(clearColorValueBlack));
            vk::RenderPassBeginInfo RPBeginInfo(m_RenderPass, m_FrameBuffers[nextImageIndex], rect2d, clearValue);
            drawCmd.beginRenderPass(RPBeginInfo, vk::SubpassContents::eInline);
            drawCmd.bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipeline);

            vk::Viewport viewport(0.f, 0.f, rect2d.extent.width, rect2d.extent.height, 0.f, 1.f);
            drawCmd.setViewport(0, viewport);

            drawCmd.setScissor(0, rect2d);

            drawCmd.draw(3, 1, 0, 0);

            drawCmd.endRenderPass();
            drawCmd.end();
        }

        auto& drawQueue = m_RenderingInfo.device.GetRenderingQueue().PresentQueue.value();

        std::array<vk::PipelineStageFlags, 1> pipelineStages{ vk::PipelineStageFlagBits::eColorAttachmentOutput };
        vk::SubmitInfo subInfo(m_GetImageSemaphores[currentFrame], pipelineStages, drawCmd, m_PresentSemaphores[currentFrame]);
        drawQueue.submit(subInfo, m_FrameFences[currentFrame]);

        vk::PresentInfoKHR presentInfo(m_PresentSemaphores[currentFrame], swapchain, nextImageIndex);
        auto result_present = drawQueue.presentKHR(presentInfo);

        currentFrame = (currentFrame + 1) % MaxFrameInFlight;
    }

} // namespace mortal
