#include "blingphong.h"

namespace mortal
{
    BlingPhong::BlingPhong(RenderingSystemInfo& info) : RenderPartBase(info)
    {
        Init();
    }

    BlingPhong::~BlingPhong()
    {
        ClearUp();
    }

    void BlingPhong::Init()
    {
        //Load Model
        m_ModelInfo = LoadObjModel("../../Asset/Model/Sphere.obj");
        //Load Texture
        auto textureInfo = LoadTexture("NahidaClip.png");
        //mvp
        auto extent2D = m_RenderingInfo.window.GetExtent2D();

        mvp.model = glm::mat4(1.0f);
        mvp.view = glm::lookAt(glm::vec3{ 5.0f, 5.0f, 5.0f }, glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{ 0.0f, 0.0f, 1.0f });
        mvp.proj = glm::perspective(glm::radians(45.f), (float)extent2D.width / (float)extent2D.height, 0.1f, 100.f);
        mvp.proj[1][1] *= -1;

        auto& device = m_RenderingInfo.device.GetDevice();
        auto& command = m_RenderingInfo.command;
        auto& graphicQueue = m_RenderingInfo.device.GetRenderingQueue().GraphicQueue.value();
        auto& swapchain = m_RenderingInfo.swapchain;
        auto depthFormatInfo = m_RenderingInfo.device.FindSupportDepthFormat(std::vector<vk::Format>{ vk::Format::eD24UnormS8Uint, vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint },
            vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);

        auto vertices_size = m_ModelInfo.vertices.size() * sizeof(m_ModelInfo.vertices[0]);
        auto indices_size = m_ModelInfo.indeices.size() * sizeof(m_ModelInfo.indeices[0]);
        //create vertex buffer index buffer and texture image include sampler and depth Image
        {
            m_VertexBuffer = CreateBufferExclusive(vertices_size, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst);
            m_IndexBuffer = CreateBufferExclusive(indices_size, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst);
            m_VertexIndexMemory = CreateMemoryAndBind_Buffer(std::vector<vk::Buffer>{ m_VertexBuffer, m_IndexBuffer }, vk::MemoryPropertyFlagBits::eDeviceLocal);
        
            vk::ImageCreateInfo textureImageCreateInfo({}, vk::ImageType::e2D, vk::Format::eR8G8B8A8Srgb, vk::Extent3D{ static_cast<uint32_t>(textureInfo.texWidth), static_cast<uint32_t>(textureInfo.texHeight), 1 },
                1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, vk::SharingMode::eExclusive);
            m_TextureImage = device.createImage(textureImageCreateInfo);
            m_TextureImageMemory = CreateMemoryAndBind_Image(m_TextureImage, vk::MemoryPropertyFlagBits::eDeviceLocal);
            vk::ImageViewCreateInfo textureImageViewCreateInfo({}, m_TextureImage, vk::ImageViewType::e2D, vk::Format::eR8G8B8A8Srgb,
                vk::ComponentMapping(vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA), 
                vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
            m_TextureImageView = device.createImageView(textureImageViewCreateInfo);
            vk::SamplerCreateInfo textureSamplerCreateInfo({}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear,
                vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat, 0.0f, VK_TRUE, 1.0f, VK_FALSE, vk::CompareOp::eAlways, 
                0.0f, 1.0f, vk::BorderColor::eFloatOpaqueBlack, VK_FALSE);
            m_TextureSampler = device.createSampler(textureSamplerCreateInfo);


            vk::ImageCreateInfo depthImageCreateInfo({}, vk::ImageType::e2D, depthFormatInfo.first, vk::Extent3D(extent2D, 1.0f), 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, 
                vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::SharingMode::eExclusive);
            m_DepthImage = device.createImage(depthImageCreateInfo);
            m_DepthImageMemory = CreateMemoryAndBind_Image(m_DepthImage, vk::MemoryPropertyFlagBits::eDeviceLocal);
            vk::ImageViewCreateInfo depthImageViewCreateInfo({}, m_DepthImage, vk::ImageViewType::e2D, depthFormatInfo.first,
                vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1));
            m_DepthImageView = device.createImageView(depthImageViewCreateInfo);

            m_MvpBuffer = CreateBufferExclusive(sizeof(MVP), vk::BufferUsageFlagBits::eUniformBuffer);
            m_MvpMemory = CreateMemoryAndBind_Buffer(std::vector<vk::Buffer>{ m_MvpBuffer }, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);
            m_MvpData = device.mapMemory(m_MvpMemory, 0, sizeof(MVP));
        }
        //copy vertex, index, image info 
        {
            vk::Buffer vertexIndexStageBuffer = CreateBufferExclusive(vertices_size + indices_size, vk::BufferUsageFlagBits::eTransferSrc);
            vk::DeviceMemory vertexIndexStageMemory = CreateMemoryAndBind_Buffer(std::vector<vk::Buffer>{ vertexIndexStageBuffer }, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
            void* data = device.mapMemory(vertexIndexStageMemory, 0, vertices_size);
            memcpy(data, m_ModelInfo.vertices.data(), vertices_size);
            device.unmapMemory(vertexIndexStageMemory);

            data = device.mapMemory(vertexIndexStageMemory, vertices_size, indices_size);
            memcpy(data, m_ModelInfo.indeices.data(), indices_size);
            device.unmapMemory(vertexIndexStageMemory);

            vk::Buffer textureStageBuffer = CreateBufferExclusive(textureInfo.dataSize, vk::BufferUsageFlagBits::eTransferSrc);
            vk::DeviceMemory textureStageMemory = CreateMemoryAndBind_Buffer(std::vector<vk::Buffer>{ textureStageBuffer }, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
            data = device.mapMemory(textureStageMemory, 0, textureInfo.dataSize);
            memcpy(data, textureInfo.data, textureInfo.dataSize);
            device.unmapMemory(textureStageMemory);

            auto cmd = command.BeginSingleCommand();
            cmd.copyBuffer(vertexIndexStageBuffer, m_VertexBuffer, { vk::BufferCopy(0, 0, vertices_size) });
            cmd.copyBuffer(vertexIndexStageBuffer, m_IndexBuffer, { vk::BufferCopy(vertices_size, 0, indices_size) });

            vk::ImageMemoryBarrier undefineToTransferDst(vk::AccessFlagBits::eNone, vk::AccessFlagBits::eTransferWrite, 
                vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, {}, {},
                m_TextureImage, vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
            cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, { undefineToTransferDst });

            cmd.copyBufferToImage(textureStageBuffer, m_TextureImage, vk::ImageLayout::eTransferDstOptimal, { vk::BufferImageCopy(0, textureInfo.texWidth, textureInfo.texHeight, 
                vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1), vk::Offset3D(0, 0, 0), vk::Extent3D(textureInfo.texWidth, textureInfo.texHeight, 1.0f))});

            vk::ImageMemoryBarrier transferDstToShaderRead(vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead,
                vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, {}, {},
                m_TextureImage, vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
            cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, { transferDstToShaderRead});

            command.EndSingleCommand(cmd, graphicQueue);

            device.freeMemory(textureStageMemory);
            device.destroyBuffer(textureStageBuffer);
            device.freeMemory(vertexIndexStageMemory);
            device.destroyBuffer(vertexIndexStageBuffer);
        }
        //Descriptor
        {
            std::vector<vk::DescriptorSetLayoutBinding> bindings{ 
                vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex),
                vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment)
            };
            vk::DescriptorSetLayoutCreateInfo MvpAndSamplerlayoutCreateInfo({}, bindings);
            m_MvpAndSamplerSetLayout = device.createDescriptorSetLayout(MvpAndSamplerlayoutCreateInfo);

            std::vector<vk::DescriptorPoolSize> poolSizes{
                vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 1),
                vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, 1)
            };
            vk::DescriptorPoolCreateInfo poolCreateInfo({}, 1, poolSizes);
            m_DescriptorPool = device.createDescriptorPool(poolCreateInfo);

            vk::DescriptorSetAllocateInfo setAllocateInfo(m_DescriptorPool, m_MvpAndSamplerSetLayout);
            m_MvpAndSamplerSets = device.allocateDescriptorSets(setAllocateInfo);

            vk::DescriptorBufferInfo MvpbufferInfo(m_MvpBuffer, 0, sizeof(MVP));
            vk::DescriptorImageInfo samplerImageInfo(m_TextureSampler, m_TextureImageView, vk::ImageLayout::eShaderReadOnlyOptimal);
            device.updateDescriptorSets({
                vk::WriteDescriptorSet(m_MvpAndSamplerSets[0], 0, 0, vk::DescriptorType::eUniformBuffer, {}, MvpbufferInfo),
                vk::WriteDescriptorSet(m_MvpAndSamplerSets[0], 1, 0, vk::DescriptorType::eCombinedImageSampler, samplerImageInfo)
            }, {});
        }
        //Set RenderPass and Framebuffer 
        {
            vk::AttachmentDescription attach_Present({}, swapchain.GetSurfaceDetail().SurfaceFormats.format, vk::SampleCountFlagBits::e1,
                vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare, 
                vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR);
            vk::ImageLayout depthLayout = vk::ImageLayout::eDepthAttachmentOptimal;
            if (depthFormatInfo.second) {
                depthLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
            }
            vk::AttachmentDescription attach_Depth({}, depthFormatInfo.first, vk::SampleCountFlagBits::e1,
                vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eDontCare, vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
                vk::ImageLayout::eUndefined, depthLayout
            );
            std::vector<vk::AttachmentDescription> attachs{ attach_Present, attach_Depth };

            vk::AttachmentReference colorRef(0, vk::ImageLayout::eColorAttachmentOptimal);
            vk::AttachmentReference depthRef(1, depthLayout);
            vk::SubpassDescription subPassDesCription({}, vk::PipelineBindPoint::eGraphics, {}, colorRef, {}, &depthRef);

            vk::SubpassDependency subPassDependency(VK_SUBPASS_EXTERNAL, 0, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput, 
                vk::AccessFlagBits::eNone, vk::AccessFlagBits::eColorAttachmentWrite);
            vk::RenderPassCreateInfo blingPhongPassCreateInfo({}, attachs, subPassDesCription, subPassDependency);
            m_BlingPhongPass = device.createRenderPass(blingPhongPassCreateInfo);
            
            auto framebufferSize = swapchain.GetSwapChainImageViews().size();
            m_Framebuffers.reserve(framebufferSize);
            for (int i = 0; i < framebufferSize; i++) {
                std::vector<vk::ImageView> attachments{
                    swapchain.GetSwapChainImageViews()[i],
                        m_DepthImageView
                };
                m_Framebuffers.emplace_back(device.createFramebuffer(
                    vk::FramebufferCreateInfo({}, m_BlingPhongPass, attachments, extent2D.width, extent2D.height, 1)
                ));
            }
        }
        //Set Pipeline
        {
            vk::VertexInputBindingDescription bindDes(0, sizeof(Vertex));
            vk::VertexInputAttributeDescription attrDes_Pos(0, 0, vk::Format::eR32G32B32Sfloat, 0);
            vk::VertexInputAttributeDescription attrDes_Color(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, Color));
            vk::VertexInputAttributeDescription attrDes_Tex(2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, TexCoord));
            vk::VertexInputAttributeDescription attrDes_Normal(3, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, Normal));
            std::vector<vk::VertexInputAttributeDescription> attrDes{
                attrDes_Pos, attrDes_Color, attrDes_Tex, attrDes_Normal
            };
            vk::PipelineVertexInputStateCreateInfo vertexInput({}, bindDes, attrDes);

            StandardGraphicPipelineInfo pipelineInfo;
            pipelineInfo.inputAssemblyState = vk::PipelineInputAssemblyStateCreateInfo({}, vk::PrimitiveTopology::eTriangleList, VK_FALSE);
            vk::Viewport viewprot(0, 0, extent2D.width, extent2D.height, 0.0f, 1.0f);
            vk::Rect2D scissor({ 0, 0 }, extent2D);
            pipelineInfo.viewportState = vk::PipelineViewportStateCreateInfo({}, viewprot, scissor);
            pipelineInfo.rasterizationState = vk::PipelineRasterizationStateCreateInfo({}, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise,
                VK_FALSE, {}, {}, {}, 1.0f);
            pipelineInfo.multisampleState = vk::PipelineMultisampleStateCreateInfo({}, vk::SampleCountFlagBits::e1, VK_FALSE);
            pipelineInfo.depthStencilState = vk::PipelineDepthStencilStateCreateInfo({}, VK_TRUE, VK_TRUE, vk::CompareOp::eLess, VK_FALSE, VK_FALSE);

            vk::PipelineColorBlendAttachmentState state(VK_FALSE, vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eOneMinusSrcAlpha, vk::BlendOp::eAdd,
                vk::BlendFactor::eZero, vk::BlendFactor::eOne, vk::BlendOp::eAdd,
                vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
            pipelineInfo.colorBlendState = vk::PipelineColorBlendStateCreateInfo({}, VK_FALSE, vk::LogicOp::eCopy, state);

            std::vector<vk::DynamicState> dynamicStates{ vk::DynamicState::eViewport, vk::DynamicState::eScissor };
            vk::PipelineDynamicStateCreateInfo dynamicState({}, dynamicStates);

            auto vertShaderModule = CreateShaderModule("BlingPhong/BlingPhong_vert");
            auto fragShaderModule = CreateShaderModule("BlingPhong/BlingPhong_frag");
            vk::PipelineShaderStageCreateInfo vertshaderStage({}, vk::ShaderStageFlagBits::eVertex, vertShaderModule, "main");
            vk::PipelineShaderStageCreateInfo fragshaderStage({}, vk::ShaderStageFlagBits::eFragment, fragShaderModule, "main");
            std::vector<vk::PipelineShaderStageCreateInfo> shaderStages{ vertshaderStage, fragshaderStage };
            
            vk::PipelineLayoutCreateInfo layoutCreateInfo({}, m_MvpAndSamplerSetLayout);
            m_BlingPhongPipelineLayout = device.createPipelineLayout(layoutCreateInfo);
            vk::GraphicsPipelineCreateInfo blingphongPipelineInfo({}, shaderStages, &vertexInput, &pipelineInfo.inputAssemblyState, nullptr, & pipelineInfo.viewportState, & pipelineInfo.rasterizationState,
                & pipelineInfo.multisampleState, & pipelineInfo.depthStencilState, & pipelineInfo.colorBlendState, &dynamicState, m_BlingPhongPipelineLayout, m_BlingPhongPass, 0, nullptr, -1);
            m_BlingPhongPipeline = device.createGraphicsPipeline({}, blingphongPipelineInfo).value;

            device.destroyShaderModule(vertShaderModule);
            device.destroyShaderModule(fragShaderModule);
        }
    }
    
    void BlingPhong::ClearUp()
    {
        auto& device = m_RenderingInfo.device.GetDevice();

        device.waitIdle();

        device.destroyPipeline(m_BlingPhongPipeline);
        device.destroyPipelineLayout(m_BlingPhongPipelineLayout);
        for (auto& framebuffer : m_Framebuffers) {
            device.destroyFramebuffer(framebuffer);
        }

        device.destroyRenderPass(m_BlingPhongPass);

        device.destroyDescriptorSetLayout(m_MvpAndSamplerSetLayout);
        device.destroyDescriptorPool(m_DescriptorPool);

        device.unmapMemory(m_MvpMemory);
        device.freeMemory(m_MvpMemory);
        device.destroyBuffer(m_MvpBuffer);

        device.destroyImageView(m_DepthImageView);
        device.freeMemory(m_DepthImageMemory);
        device.destroyImage(m_DepthImage);

        device.destroySampler(m_TextureSampler);
        device.destroyImageView(m_TextureImageView);
        device.freeMemory(m_TextureImageMemory);
        device.destroyImage(m_TextureImage);

        device.freeMemory(m_VertexIndexMemory);
        device.destroyBuffer(m_IndexBuffer);
        device.destroyBuffer(m_VertexBuffer);
    }

    void BlingPhong::Draw()
    {
        {
            static auto start = std::chrono::high_resolution_clock::now();
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration<float, std::chrono::seconds::period>(end - start).count();
            mvp.model = glm::rotate(glm::mat4(1.0f), glm::radians(duration * 90.f), glm::vec3(0.0f, 0.0f, 1.0f)) *
                glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            memcpy(m_MvpData, &mvp, sizeof(mvp));
        }

        auto& drawCmd = m_RenderingInfo.command.GetCommandBuffers()[m_RenderingInfo.CurrentFrame];
        auto extent2D = m_RenderingInfo.window.GetExtent2D();
        drawCmd.begin(vk::CommandBufferBeginInfo());

        std::vector<vk::ClearValue> clearValues{ 
            vk::ClearValue(vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f})), 
            vk::ClearValue(vk::ClearDepthStencilValue(1.0f, 0))
        };
        vk::RenderPassBeginInfo rpBeginInfo(m_BlingPhongPass, m_Framebuffers[m_RenderingInfo.nextImageIndex], vk::Rect2D({0, 0}, extent2D), clearValues);
        drawCmd.beginRenderPass(rpBeginInfo, vk::SubpassContents::eInline);

        drawCmd.setViewport(0, { vk::Viewport(0, 0, extent2D.width, extent2D.height, 0.0f, 1.0f)});
        drawCmd.setScissor(0, { vk::Rect2D({0, 0}, extent2D)});
        drawCmd.bindPipeline(vk::PipelineBindPoint::eGraphics, m_BlingPhongPipeline);

        drawCmd.bindVertexBuffers(0, { m_VertexBuffer }, {0});
        drawCmd.bindIndexBuffer(m_IndexBuffer, 0, vk::IndexType::eUint32);

        drawCmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_BlingPhongPipelineLayout, 0, m_MvpAndSamplerSets, {});
        drawCmd.drawIndexed(m_ModelInfo.indeices.size(), 1, 0, 0, 0);

        drawCmd.endRenderPass();
        drawCmd.end();
    }
} // namespace mortal
