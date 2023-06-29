#include "triangle.h"
#include "Rendering/rendering_camera.h"

namespace mortal
{
    TrianglePart::TrianglePart(VulkanContext& info) : RenderPartBase(info)
    {
        Init();
    }
    TrianglePart::~TrianglePart()
    {
        ClearUp();
    }

    void TrianglePart::Init()
    {
        auto& device = m_RenderingInfo.device.GetDevice();

        auto extent2d = m_RenderingInfo.window.GetExtent2D();

        vk::Format depthFormat;
        // Set Buffer and Memory
        {
        //Set data
            Vertex v1;
            v1.Position = glm::vec3(0.0f, -0.5f, 0.0f);
            v1.Color = glm::vec3(255.0f, 255.0f, 255.0f);
            v1.Normal = glm::vec3(1.0f, 0.0f, 0.0f);
            v1.TexCoord = glm::vec2(0.0f, 0.0f);

            Vertex v2;
            v2.Position = glm::vec3(0.0f, 0.5f, 0.0f);
            v2.Color = glm::vec3(255.0f, 255.0f, 255.0f);
            v2.Normal = glm::vec3(1.0f, 0.0f, 0.0f);
            v2.TexCoord = glm::vec2(1.0f, 0.0f);

            Vertex v3;
            v3.Position = glm::vec3(0.0f, 0.0f, 1.0f);
            v3.Color = glm::vec3(255.0f, 255.0f, 255.0f);
            v3.Normal = glm::vec3(1.0f, 0.0f, 0.0f);
            v3.TexCoord = glm::vec2(0.5f, 1.0f);

            std::vector<Vertex> vertices = {
                v1, v2, v3
            };

            std::vector<IndexType> indices = {
                0, 1, 2
            };
            Test_Vertices = vertices;
            Test_Indices = indices;

            mvp.Model = glm::mat4(1.0f);
            //mvp.View = glm::lookAt(glm::vec3{ 5.0f, 5.0f, 5.0f }, glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{0.0f, 0.0f, 1.0f});
            mvp.View = m_RenderingInfo.m_Camera.GetView();
            mvp.Project = glm::perspective(glm::radians(45.f), (float)extent2d.width / (float) extent2d.height, 0.1f, 100.f);
            mvp.Project[1][1] *= -1;

            TextureInfo textureInfo = LoadTexture("NahidaClip.png");
        //set buffer and memory
            auto vertex_size = sizeof(Vertex) * Test_Vertices.size();
            auto index_size = sizeof(IndexType) * Test_Indices.size();

            vk::BufferCreateInfo VertexBufferCI({}, vertex_size, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::SharingMode::eExclusive);
            m_VertexBuffer = device.createBuffer(VertexBufferCI);
            
            vk::BufferCreateInfo IndexBufferCI({}, index_size, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::SharingMode::eExclusive);
            m_IndexBuffer = device.createBuffer(IndexBufferCI);

            m_VertexIndexMemroy = CreateMemoryAndBind_Buffer(std::vector<vk::Buffer>{ m_VertexBuffer, m_IndexBuffer }, vk::MemoryPropertyFlagBits::eDeviceLocal);
            
            //Set UniformBuffer
            auto mvp_size = sizeof(mvp);
            vk::BufferCreateInfo createInfo({}, mvp_size, vk::BufferUsageFlagBits::eUniformBuffer, vk::SharingMode::eExclusive);
            m_MVPUniformBuffer = device.createBuffer(createInfo);

            m_MVPMemory = CreateMemoryAndBind_Buffer(std::vector<vk::Buffer>{ m_MVPUniformBuffer }, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
            m_MVPData = device.mapMemory(m_MVPMemory, 0, mvp_size);

        //stage buffer and copy data
            //Copy Vertex Buffer
            {
                vk::BufferCreateInfo StageBufferCI({}, vertex_size, vk::BufferUsageFlagBits::eTransferSrc, vk::SharingMode::eExclusive);
                vk::Buffer StageBuffer = device.createBuffer(StageBufferCI);

                auto StageMemory = CreateMemoryAndBind_Buffer(std::vector<vk::Buffer>{ StageBuffer },
                    vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

                void* data = device.mapMemory(StageMemory, 0, vertex_size);
                memcpy(data, Test_Vertices.data(), vertex_size);
                device.unmapMemory(StageMemory);

                vk::CommandBuffer SingleCmdBuffer = m_RenderingInfo.command.BeginSingleCommand();
                vk::BufferCopy vertexBC(0, 0, vertex_size);
                SingleCmdBuffer.copyBuffer(StageBuffer, m_VertexBuffer, vertexBC);
                m_RenderingInfo.command.EndSingleCommand(SingleCmdBuffer, m_RenderingInfo.device.GetRenderingQueue().GraphicQueue.value());

                device.freeMemory(StageMemory);
                device.destroyBuffer(StageBuffer);
            }
            
            //Copy Index Buffer
            {
                vk::BufferCreateInfo StageBufferCI({}, index_size, vk::BufferUsageFlagBits::eTransferSrc, vk::SharingMode::eExclusive);
                vk::Buffer StageBuffer = device.createBuffer(StageBufferCI);

                auto StageMemory = CreateMemoryAndBind_Buffer(std::vector<vk::Buffer>{StageBuffer},
                    vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

                void* data = device.mapMemory(StageMemory, 0, index_size);
                memcpy(data, Test_Indices.data(), index_size);
                device.unmapMemory(StageMemory);

                vk::CommandBuffer SingleCmdBuffer = m_RenderingInfo.command.BeginSingleCommand();
                vk::BufferCopy indexBC(0, 0, index_size);
                SingleCmdBuffer.copyBuffer(StageBuffer, m_IndexBuffer, indexBC);
                m_RenderingInfo.command.EndSingleCommand(SingleCmdBuffer, m_RenderingInfo.device.GetRenderingQueue().GraphicQueue.value());

                device.freeMemory(StageMemory);
                device.destroyBuffer(StageBuffer);
            }

            //Set Texture Image And Copy (important)
            {
                vk::ImageCreateInfo createInfo({}, vk::ImageType::e2D, vk::Format::eR8G8B8A8Srgb, vk::Extent3D(textureInfo.texWidth, textureInfo.texHeight, 1.0f), 1, 1, vk::SampleCountFlagBits::e1, 
                    vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, vk::SharingMode::eExclusive, {},
                    vk::ImageLayout::eUndefined);
                m_TextureImage = device.createImage(createInfo);
                
                m_TextureMemory = CreateMemoryAndBind_Image(m_TextureImage, vk::MemoryPropertyFlagBits::eDeviceLocal);

                vk::BufferCreateInfo StagebufferInfo({}, textureInfo.dataSize, vk::BufferUsageFlagBits::eTransferSrc, vk::SharingMode::eExclusive);
                auto stageBuffer = device.createBuffer(StagebufferInfo);
                auto stageMemory = CreateMemoryAndBind_Buffer(std::vector<vk::Buffer>{ stageBuffer }, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

                void* textureData = device.mapMemory(stageMemory, 0, textureInfo.dataSize);
                memcpy(textureData, textureInfo.data, textureInfo.dataSize);
                device.unmapMemory(stageMemory);

                auto sCmd = m_RenderingInfo.command.BeginSingleCommand();

                vk::ImageMemoryBarrier texToTransfer({}, vk::AccessFlagBits::eTransferWrite, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, 
                    {}, {}, m_TextureImage, vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
                sCmd.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, { texToTransfer });

                vk::BufferImageCopy textureRegions (0, textureInfo.texWidth, textureInfo.texHeight, vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1), 
                    vk::Offset3D(0, 0, 0), vk::Extent3D(textureInfo.texWidth, textureInfo.texHeight, 1));
                sCmd.copyBufferToImage(stageBuffer, m_TextureImage, vk::ImageLayout::eTransferDstOptimal, { textureRegions });

                vk::ImageMemoryBarrier texFromTransferToReadInFrag(vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead, 
                    vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, m_TextureImage, 
                    vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
                sCmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, { texFromTransferToReadInFrag });

                m_RenderingInfo.command.EndSingleCommand(sCmd, m_RenderingInfo.device.GetRenderingQueue().GraphicQueue.value());

                device.freeMemory(stageMemory);
                device.destroyBuffer(stageBuffer);

                vk::ImageViewCreateInfo ivCreateInfo({}, m_TextureImage, vk::ImageViewType::e2D, vk::Format::eR8G8B8A8Srgb,
                    vk::ComponentMapping(vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA), 
                    vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
                m_TextureImageView = device.createImageView(ivCreateInfo);

                vk::SamplerCreateInfo sCreateInfo({}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear,
                    vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat, 0.0f, 
                    VK_TRUE, m_RenderingInfo.device.GetPhysicalDeviceProperties().limits.maxSamplerAnisotropy, 
                    VK_FALSE, vk::CompareOp::eAlways, 
                    0.0f, 0.0f, vk::BorderColor::eFloatOpaqueBlack, VK_FALSE);
                m_TextureSampler = device.createSampler(sCreateInfo);
            }

            //Set Depth Image
            {
                auto depthFormats = m_RenderingInfo.device.FindSupportDepthFormat(std::vector<vk::Format>{ vk::Format::eD24UnormS8Uint, vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint }, 
                    vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
                depthFormat = depthFormats.first;
                SupportStencil = depthFormats.second;
                vk::ImageCreateInfo depthImageCreateInfo({}, vk::ImageType::e2D, depthFormat, vk::Extent3D(extent2d, 1.0f), 1, 1, vk::SampleCountFlagBits::e1,
                    vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::SharingMode::eExclusive);
                m_DepthImage = device.createImage(depthImageCreateInfo);

                m_DepthImageMemory = CreateMemoryAndBind_Image(m_DepthImage, vk::MemoryPropertyFlagBits::eDeviceLocal);

                vk::ImageViewCreateInfo depthIVCreateInfo({}, m_DepthImage, vk::ImageViewType::e2D, depthFormat,
                    vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1));
                m_DepthImageView = device.createImageView(depthIVCreateInfo);
            }

            //Set DesCriptorSetLayout
            {
                std::vector<vk::DescriptorSetLayoutBinding> bindings;
                bindings.reserve(3);
                vk::DescriptorSetLayoutBinding Binding_Mvp(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex);
                bindings.emplace_back(Binding_Mvp);

                vk::DescriptorSetLayoutBinding Binding_Sampler(1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment);
                bindings.emplace_back(Binding_Sampler);

                vk::DescriptorSetLayoutCreateInfo CreateInfo({}, bindings);
                m_TriangleDescriptorSetLayout =  device.createDescriptorSetLayout(CreateInfo);
            }

            //Create descriptor pool
            {
                std::vector<vk::DescriptorPoolSize> poolSize{
                    vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 1),
                    vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, 1)
                };
                vk::DescriptorPoolCreateInfo CreateInfo(vk::DescriptorPoolCreateFlagBits{}, 1, poolSize);
                m_DescriptorPool = device.createDescriptorPool(CreateInfo);
            }

            //Allocate descriptor Sets
            {
                vk::DescriptorSetAllocateInfo SetsAllInfo(m_DescriptorPool, m_TriangleDescriptorSetLayout);
                m_TriangleDesCriptorSets = device.allocateDescriptorSets(SetsAllInfo);
            }

            //Update Descriptor Set
            {
                std::vector<vk::WriteDescriptorSet> writeSets;
                writeSets.reserve(3);
                for (auto& desCriptorSet : m_TriangleDesCriptorSets) {
                    vk::DescriptorBufferInfo MVPBufferInfo(m_MVPUniformBuffer, 0, sizeof(UBO));
                    vk::WriteDescriptorSet writeMVP(desCriptorSet, 0, 0, vk::DescriptorType::eUniformBuffer, {}, MVPBufferInfo);
                    writeSets.emplace_back(writeMVP);

                    vk::DescriptorImageInfo SampleImageInfo(m_TextureSampler, m_TextureImageView, vk::ImageLayout::eShaderReadOnlyOptimal);
                    vk::WriteDescriptorSet writeSampler(desCriptorSet, 1, 0, vk::DescriptorType::eCombinedImageSampler, SampleImageInfo);
                    writeSets.emplace_back(writeSampler);
                }

                device.updateDescriptorSets(writeSets, {});
            }
        }

        //create pipeline layout
        {
            vk::PipelineLayoutCreateInfo createInfo({}, m_TriangleDescriptorSetLayout);
            m_PipelineLayout = device.createPipelineLayout(createInfo);
        }

        //set RenderPass
        {
            //About framebuffer description
            auto format = m_RenderingInfo.swapchain.GetSurfaceDetail().SurfaceFormats.format;

            vk::ImageLayout depthLayout = vk::ImageLayout::eDepthAttachmentOptimal;
            if (SupportStencil) {
                depthLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
            }

            vk::AttachmentDescription attachDes({}, format, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, 
                vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR);
            vk::AttachmentDescription depthAttachDes({}, depthFormat, vk::SampleCountFlagBits::e1, vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eDontCare,
                vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare, vk::ImageLayout::eUndefined, depthLayout);
            std::array<vk::AttachmentDescription, 2> attachDescriptions{ attachDes, depthAttachDes };

            //subPassDescription
            vk::AttachmentReference colorAttachRef(0, vk::ImageLayout::eColorAttachmentOptimal);

            vk::AttachmentReference depthAttachRef(1, depthLayout);

            vk::SubpassDescription subPassDes({}, vk::PipelineBindPoint::eGraphics, {}, colorAttachRef, {}, &depthAttachRef);

            vk::SubpassDependency dependency(VK_SUBPASS_EXTERNAL, 0, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput,
                vk::AccessFlagBits::eNone, vk::AccessFlagBits::eColorAttachmentWrite);

            vk::RenderPassCreateInfo renderPassCreateInfo({}, attachDescriptions, subPassDes, dependency);
            m_RenderPass = device.createRenderPass(renderPassCreateInfo);
        }

        //Set Framebuffer
        {
            auto& imageViews = m_RenderingInfo.swapchain.GetSwapChainImageViews();
            auto size = imageViews.size();
            m_FrameBuffers.resize(size);
            for (size_t i = 0; i < size; i++) {
                std::vector<vk::ImageView> imageViewNeeds{ 
                    imageViews[i], 
                    m_DepthImageView
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
            //-------m_VertexShaderModule = CreateShaderModule("triangle/triangle_vert");


            auto bCodeFrag = LoadShader("triangle/triangle_frag");
            vk::ShaderModuleCreateInfo fragCreateInfo({}, bCodeFrag.size(), reinterpret_cast<uint32_t*>(bCodeFrag.data()));
            m_FragmentShaderModule = device.createShaderModule(fragCreateInfo);
            //-------m_FragmentShaderModule = CreateShaderModule("triangle/triangle_frag");
        }

        //create pipeline
        {
            //shader stage
            vk::PipelineShaderStageCreateInfo vertStageCreateInfo({}, vk::ShaderStageFlagBits::eVertex, m_VertexShaderModule, "main");
            vk::PipelineShaderStageCreateInfo fragStageCreateInfo({}, vk::ShaderStageFlagBits::eFragment, m_FragmentShaderModule, "main");

            std::array<vk::PipelineShaderStageCreateInfo, 2> StageCreateInfos{ vertStageCreateInfo, fragStageCreateInfo };

            //vertex input state
            vk::VertexInputBindingDescription bindDes(0, sizeof(Vertex), vk::VertexInputRate::eVertex);

            vk::VertexInputAttributeDescription attrDes_position(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, Position));
            vk::VertexInputAttributeDescription attrDes_color(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, Color));
            vk::VertexInputAttributeDescription attrDes_TexCoord(2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, TexCoord));
            vk::VertexInputAttributeDescription attrDes_normal(3, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, Normal));

            std::array<vk::VertexInputAttributeDescription, 4> attrDes{ attrDes_position, attrDes_color, attrDes_TexCoord, attrDes_normal };
            vk::PipelineVertexInputStateCreateInfo vertInputStateInfo({}, bindDes, attrDes);

            //input assembly state
            vk::PipelineInputAssemblyStateCreateInfo inputAssStateInfo({}, vk::PrimitiveTopology::eTriangleList, VK_FALSE);

            //viewport state
            
            vk::Viewport viewport(0, 0, extent2d.width, extent2d.height, 0.f, 1.f);
            vk::Rect2D scissor({}, extent2d);
            vk::PipelineViewportStateCreateInfo viewportStateInfo({}, viewport, scissor);

            //rasterization state
            vk::PipelineRasterizationStateCreateInfo rasterStateInfo({}, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise, VK_FALSE, 0.0f, 0.0f, 0.0f, 1.0f);

            //multisample state
            vk::PipelineMultisampleStateCreateInfo multiSampleStateInfo({}, vk::SampleCountFlagBits::e1, VK_FALSE);

            //depth stencil state
            vk::PipelineDepthStencilStateCreateInfo depthStencilStateInfo({}, VK_TRUE, VK_TRUE, vk::CompareOp::eLess, VK_FALSE, VK_FALSE);

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
    }

    void TrianglePart::ClearUp()
    {
        auto& device = m_RenderingInfo.device.GetDevice();
        device.waitIdle();

        device.destroyDescriptorPool(m_DescriptorPool);
        device.destroyDescriptorSetLayout(m_TriangleDescriptorSetLayout);
        
        device.destroyImageView(m_DepthImageView);
        device.freeMemory(m_DepthImageMemory);
        device.destroyImage(m_DepthImage);

        device.destroySampler(m_TextureSampler);

        device.destroyImageView(m_TextureImageView);
        device.freeMemory(m_TextureMemory);
        device.destroyImage(m_TextureImage);

        device.unmapMemory(m_MVPMemory);
        device.freeMemory(m_MVPMemory);
        device.destroyBuffer(m_MVPUniformBuffer);

        device.freeMemory(m_VertexSecondMemory);
        device.destroyBuffer(m_VertexBufferSecond);

        device.freeMemory(m_VertexIndexMemroy);
        device.destroyBuffer(m_IndexBuffer);
        device.destroyBuffer(m_VertexBuffer);

        device.destroyPipeline(m_Pipeline);
        device.destroyShaderModule(m_FragmentShaderModule);
        device.destroyShaderModule(m_VertexShaderModule);
        for (auto& framebuffer : m_FrameBuffers) {
            device.destroyFramebuffer(framebuffer);
        }
        device.destroyRenderPass(m_RenderPass);
        device.destroyPipelineLayout(m_PipelineLayout);
    }

    void mortal::TrianglePart::Draw()
    {
        PrepareFrame();
        //Record Draw Cmd
        {
            auto& drawCmd = m_RenderingInfo.command.GetCommandBuffers()[m_RenderingInfo.CurrentFrame];
            vk::CommandBufferBeginInfo beginInfo{};
            drawCmd.begin(beginInfo);

            vk::Rect2D rect2d({}, m_RenderingInfo.window.GetExtent2D());

            vk::ClearValue clearValue;
            std::array<float, 4> clearColorValueBlack{ 0.0f, 0.0f, 0.0f, 1.0f };
            clearValue.setColor(vk::ClearColorValue(clearColorValueBlack));
            vk::ClearValue clearDepthValue;
            clearDepthValue.setDepthStencil(vk::ClearDepthStencilValue(1.0f, 0));

            std::array<vk::ClearValue, 2> clearValues{ clearValue, clearDepthValue };

            vk::RenderPassBeginInfo RPBeginInfo(m_RenderPass, m_FrameBuffers[m_RenderingInfo.nextImageIndex], rect2d, clearValues);
            drawCmd.beginRenderPass(RPBeginInfo, vk::SubpassContents::eInline);
            drawCmd.bindPipeline(vk::PipelineBindPoint::eGraphics, m_Pipeline);

            vk::Viewport viewport(0.f, 0.f, rect2d.extent.width, rect2d.extent.height, 0.f, 1.f);
            drawCmd.setViewport(0, viewport);

            drawCmd.setScissor(0, rect2d);

            drawCmd.bindVertexBuffers(0, m_VertexBuffer, {0});
            drawCmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_PipelineLayout, 0, m_TriangleDesCriptorSets, {});
            drawCmd.bindIndexBuffer(m_IndexBuffer, 0, vk::IndexType::eUint16);
            drawCmd.drawIndexed(Test_Indices.size(), 1, 0, 0, 0);

            drawCmd.endRenderPass();
            drawCmd.end();
        }

        //Updata MVP
        {
            mvp.View = m_RenderingInfo.m_Camera.GetView();
            memcpy(m_MVPData, &mvp, sizeof(mvp));
        }

        SubmitQueueSync();
    }

} // namespace mortal
