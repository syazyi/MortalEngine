#include "deferred.h"

namespace mortal
{
    DeferredPart::DeferredPart(VulkanContext& info) : RenderPartBase(info)
    {
        Init();
    }

    DeferredPart::~DeferredPart()
    {
        ClearUp();
    }

    void DeferredPart::Init()
    {
        auto& device = m_RenderingInfo.device.GetDevice();
        auto extent2D = m_RenderingInfo.window.GetExtent2D();
        {
            m_ModelPlane = PrepareModel("../../Asset/Model/Plane.obj");
            //m_ModelMonkeyHead = PrepareModel("../../Asset/Model/MonkeyHeadFaceXAxis.obj");
            m_ModelUBO = PrepareUniform<ModelUBO>();
            m_ModelUBOData.model = GetBlendCorrectionModelMat();
            m_ModelUBOData.proj = glm::perspective(glm::radians(45.f), (float)extent2D.width / (float)extent2D.height, 0.1f, 100.f);
            m_ModelUBOData.proj[1][1] *= -1.f;
        }
        //To DO: 加入了两个纹理，需要导入这两个纹理信息。先渲染一个平面试试。目前只加入了一个Pass， 还要加另外一个Pass，用于离屏渲染。
        //离屏渲染用来将一些所要的位置，法线，颜色信息绘制到场景中。这里不绘制光源。
        //第二个Pass再利用传入的信息绘制光源。

        auto normal_texture_info = PrepareTexture("Deffered/stonefloor01_normal_rgba.png");
        auto albedo_texture_info = PrepareTexture("Deffered/stonefloor01_color_rgba.png");

        //Descriptor
        {
            std::vector<vk::DescriptorPoolSize> poolsizes = {
                vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 1),
                vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, 2)
            };
            m_DescriptorPool = device.createDescriptorPool(vk::DescriptorPoolCreateInfo({}, 1, poolsizes));
            std::vector<vk::DescriptorSetLayoutBinding> bindings = {
                vk::DescriptorSetLayoutBinding{0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex},
                vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment, normal_texture_info.textureSampler), 
                vk::DescriptorSetLayoutBinding(2, vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment, albedo_texture_info.textureSampler)
            };
            m_DescriptorSetLayout = device.createDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo({}, bindings));
            m_PresentDescriptorSets = device.allocateDescriptorSets(vk::DescriptorSetAllocateInfo(m_DescriptorPool, m_DescriptorSetLayout));
            auto presentBufferInfo = vk::DescriptorBufferInfo(m_ModelUBO.uniformBuffer, 0, sizeof(ModelUBO));
            auto normalDecriptorImageInfo = vk::DescriptorImageInfo(normal_texture_info.textureSampler, normal_texture_info.textureImageView, vk::ImageLayout::eShaderReadOnlyOptimal);
            auto albedoDecriptorImageInfo = vk::DescriptorImageInfo(albedo_texture_info.textureSampler, albedo_texture_info.textureImageView, vk::ImageLayout::eShaderReadOnlyOptimal);
            device.updateDescriptorSets({
                    vk::WriteDescriptorSet(m_PresentDescriptorSets[0], 0, 0, vk::DescriptorType::eUniformBuffer, {}, presentBufferInfo), 
                    vk::WriteDescriptorSet(m_PresentDescriptorSets[0], 1, 0, vk::DescriptorType::eCombinedImageSampler, normalDecriptorImageInfo),
                    vk::WriteDescriptorSet(m_PresentDescriptorSets[0], 2, 0, vk::DescriptorType::eCombinedImageSampler, albedoDecriptorImageInfo),
                }, {});
        }

        auto swapchain = m_RenderingInfo.swapchain;
        //RenderPass Framebuffer
        {
            std::vector<vk::AttachmentDescription> attachDes = {
                vk::AttachmentDescription({}, swapchain.GetSurfaceDetail().SurfaceFormats.format, vk::SampleCountFlagBits::e1, 
                vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare, 
                vk::ImageLayout::eUndefined,  vk::ImageLayout::ePresentSrcKHR)
            };
            vk::AttachmentReference colorAttachment(0, vk::ImageLayout::eColorAttachmentOptimal);
            vk::SubpassDescription subpassDescruption({}, vk::PipelineBindPoint::eGraphics, {}, colorAttachment);
            vk::SubpassDependency subpassDependency(VK_SUBPASS_EXTERNAL, 0, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput,
                vk::AccessFlagBits::eNone, vk::AccessFlagBits::eColorAttachmentWrite);

            // RenderCreateInfo2 createRenderPass2
            m_PresentRenderPass = device.createRenderPass(vk::RenderPassCreateInfo({}, attachDes, subpassDescruption, subpassDependency));

            for (auto& swapchainImage : m_RenderingInfo.swapchain.GetSwapChainImageViews()) {
                std::vector<vk::ImageView> frameImageView = {
                    swapchainImage
                };
                m_PresentFrameBuffers.emplace_back(device.createFramebuffer(vk::FramebufferCreateInfo({}, m_PresentRenderPass, frameImageView, extent2D.width, extent2D.height, 1)));
            }
        }

        //Pipeline
        {
            vk::PipelineLayoutCreateInfo PresentPipelineLayoutCI({}, m_DescriptorSetLayout);
            m_PresentPipelineLayout = device.createPipelineLayout(PresentPipelineLayoutCI);

            std::vector<vk::VertexInputBindingDescription> present_VIBDs = {
                vk::VertexInputBindingDescription(0, sizeof(Vertex), vk::VertexInputRate::eVertex)
            };
            std::vector<vk::VertexInputAttributeDescription> present_VIADs = {
                vk::VertexInputAttributeDescription(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, Position)),
                vk::VertexInputAttributeDescription(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, Color)),
                vk::VertexInputAttributeDescription(2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, TexCoord)),
                vk::VertexInputAttributeDescription(3, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, Normal))
            };
            vk::PipelineVertexInputStateCreateInfo VertexInputInfo({}, present_VIBDs, present_VIADs);
            vk::PipelineInputAssemblyStateCreateInfo InputAssemblyCI({}, vk::PrimitiveTopology::eTriangleList);

            vk::Viewport viewport(0.0f, 0.0f, extent2D.width, extent2D.height, 0.0f, 1.0f);
            vk::Rect2D scissor({0, 0}, extent2D);
            vk::PipelineViewportStateCreateInfo ViewportStateCI({}, viewport, scissor);
            vk::PipelineRasterizationStateCreateInfo RasterizationStateCI({}, 0, 0, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise,
                0, 0.0f, 0.0f, 0.0f, 1.0f);
            vk::PipelineDepthStencilStateCreateInfo DepthStencilStateCI({}, 1U, 1U, vk::CompareOp::eLess, 0U, 0U);

            std::vector<vk::PipelineColorBlendAttachmentState> ColorBlendAS = {
                vk::PipelineColorBlendAttachmentState(0, vk::BlendFactor::eOne, vk::BlendFactor::eOne, vk::BlendOp::eAdd,
                vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eDstAlpha, vk::BlendOp::eAdd,
                vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
            };
            vk::PipelineColorBlendStateCreateInfo ColorBlendStateCI({}, 0U, vk::LogicOp::eAnd, ColorBlendAS);

            std::vector<vk::DynamicState> DynamicStates = {
                vk::DynamicState::eViewport, 
                vk::DynamicState::eScissor
            };
            vk::PipelineDynamicStateCreateInfo DynamicStateCI({}, DynamicStates);

        //Present Pipeline
            vk::ShaderModule PresentVertexShaderModule =  CreateShaderModule("Deferred/Deffered_vert");
            vk::ShaderModule PresentFragmentShaderModule =  CreateShaderModule("Deferred/Deffered_frag");

            std::vector<vk::PipelineShaderStageCreateInfo> ShaderStateCIs = {
                vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eVertex, PresentVertexShaderModule, "main"),
                vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eFragment, PresentFragmentShaderModule, "main")
            };

            vk::GraphicsPipelineCreateInfo presentPipelineCreateInfo({}, ShaderStateCIs, &VertexInputInfo, &InputAssemblyCI, nullptr, &ViewportStateCI, &RasterizationStateCI, nullptr,
                & DepthStencilStateCI, & ColorBlendStateCI, &DynamicStateCI, m_PresentPipelineLayout, m_PresentRenderPass, 0, VK_NULL_HANDLE, 0);
            m_PresentPipeline = device.createGraphicsPipeline(nullptr, presentPipelineCreateInfo).value;

            device.destroyShaderModule(PresentFragmentShaderModule);
            device.destroyShaderModule(PresentVertexShaderModule);
        }
    }

    void DeferredPart::Draw()
    {
        PrepareFrame();
        auto& device = m_RenderingInfo.device.GetDevice();
        auto CurrentFrame = m_RenderingInfo.CurrentFrame;
        auto drawCmd = m_RenderingInfo.command.GetCommandBuffers()[CurrentFrame];
        auto extent2D = m_RenderingInfo.window.GetExtent2D();
        {
            m_ModelUBOData.view = m_RenderingInfo.m_Camera.GetView();
            memcpy(m_ModelUBO.mapped, &m_ModelUBOData, sizeof(m_ModelUBOData));
        }

    //Draw
        {
            drawCmd.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));

            std::vector<vk::ClearValue> ClearValues = {
                vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f})
            };
            drawCmd.beginRenderPass(vk::RenderPassBeginInfo(m_PresentRenderPass, m_PresentFrameBuffers[m_RenderingInfo.nextImageIndex], 
                vk::Rect2D({0, 0}, extent2D), ClearValues), vk::SubpassContents::eInline);
            drawCmd.bindPipeline(vk::PipelineBindPoint::eGraphics, m_PresentPipeline);
            drawCmd.setViewport(0, {vk::Viewport(0.0f, 0.0f, extent2D.width, extent2D.height, 0.0f, 1.0f)});
            drawCmd.setScissor(0, { vk::Rect2D({0, 0}, extent2D) });
            drawCmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_PresentPipelineLayout, 0, { m_PresentDescriptorSets }, {});
         //Draw monkeyhead
            /*drawCmd.bindVertexBuffers(0, m_ModelMonkeyHead.vertexBuffer, { 0 });
            drawCmd.bindIndexBuffer(m_ModelMonkeyHead.indexBuffer, 0, vk::IndexType::eUint32);
            drawCmd.drawIndexed(m_ModelMonkeyHead.modelInfo.indeices.size(), 1, 0, 0, 0);*/
        //Draw Plane
            drawCmd.bindVertexBuffers(0, m_ModelPlane.vertexBuffer, { 0 });
            drawCmd.bindIndexBuffer(m_ModelPlane.indexBuffer, 0, vk::IndexType::eUint32);
            drawCmd.drawIndexed(m_ModelPlane.modelInfo.indeices.size(), 1, 0, 0, 0);

            drawCmd.endRenderPass();
            drawCmd.end();
        }
        SubmitQueueSync();
    }

    void DeferredPart::ClearUp()
    {
        auto& device = m_RenderingInfo.device.GetDevice();
        device.waitIdle();

        device.destroyPipelineLayout(m_PresentPipelineLayout);
        device.destroyPipeline(m_PresentPipeline);

        for (auto& framebuffer : m_PresentFrameBuffers) {
            device.destroyFramebuffer(framebuffer);
        }
        device.destroyRenderPass(m_PresentRenderPass);

        device.destroyDescriptorSetLayout(m_DescriptorSetLayout);
        device.destroyDescriptorPool(m_DescriptorPool);

        ClearUpPrepareUniform(m_ModelUBO);
        ClearUpPrepareModel(m_ModelMonkeyHead);
        ClearUpPrepareModel(m_ModelPlane);
    }

} // namespace mortal
