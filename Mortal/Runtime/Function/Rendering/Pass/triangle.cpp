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

        auto extent2d = m_RenderingInfo.window.GetExtent2D();
        // Set Buffer and Memory
        {
        //Set data
            Test_Vertices = {
                //0
                {{0.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
                //1
                {{0.0f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
                //2
                {{0.5f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
                //3
                {{0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
            };
            Test_Indices = {
                0, 2, 1, 
                2, 3, 1
            };

            mvp.Model = glm::mat4(1.0f);
            mvp.View = glm::lookAt(glm::vec3{ 2.0f, 2.0f, 2.0f }, glm::vec3{ 0.0f, 0.0f, 0.0f }, glm::vec3{0.0f, 0.0f, 1.0f});
            mvp.Project = glm::perspective(glm::radians(45.f), (float)extent2d.width / (float) extent2d.height, 0.1f, 100.f);
            mvp.Project[1][1] *= -1;

            TextureInfo textureInfo = LoadTexture("NahidaClip.png");
        //set buffer and memory
            auto vertex_size = sizeof(Vertex) * Test_Vertices.size();
            auto index_size = sizeof(uint32_t) * Test_Indices.size();

            vk::BufferCreateInfo VertexBufferCI({}, vertex_size, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::SharingMode::eExclusive);
            m_VertexBuffer = device.createBuffer(VertexBufferCI);
            
            vk::BufferCreateInfo IndexBufferCI({}, index_size, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::SharingMode::eExclusive);
            m_IndexBuffer = device.createBuffer(IndexBufferCI);

            m_VertexIndexMemroy = CreateMemoryAndBind(std::vector<vk::Buffer>{ m_VertexBuffer, m_IndexBuffer }, vk::MemoryPropertyFlagBits::eDeviceLocal);

            //Set UniformBuffer
            auto mvp_size = sizeof(mvp);
            vk::BufferCreateInfo createInfo({}, mvp_size, vk::BufferUsageFlagBits::eUniformBuffer, vk::SharingMode::eExclusive);
            m_MVPUniformBuffer = device.createBuffer(createInfo);

            m_MVPMemory = CreateMemoryAndBind(std::vector<vk::Buffer>{ m_MVPUniformBuffer }, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
            m_MVPData = device.mapMemory(m_MVPMemory, 0, mvp_size);

        //stage buffer and copy data
            //Copy Vertex Buffer
            {
                vk::BufferCreateInfo StageBufferCI({}, vertex_size, vk::BufferUsageFlagBits::eTransferSrc, vk::SharingMode::eExclusive);
                vk::Buffer StageBuffer = device.createBuffer(StageBufferCI);

                auto StageMemory = CreateMemoryAndBind(std::vector<vk::Buffer>{StageBuffer}, 
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

                auto StageMemory = CreateMemoryAndBind(std::vector<vk::Buffer>{StageBuffer},
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

            //Set Texture Image
            {
                vk::ImageCreateInfo createInfo({}, vk::ImageType::e2D, vk::Format::eR8G8B8A8Srgb, vk::Extent3D(textureInfo.texWidth, textureInfo.texHeight, 1.0f), 1, 1, vk::SampleCountFlagBits::e1, 
                    vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, vk::SharingMode::eExclusive, {},
                    vk::ImageLayout::eUndefined);
                m_TextureImage = device.createImage(createInfo);

                vk::MemoryRequirements requires;
                requires = device.getImageMemoryRequirements(m_TextureImage);

                uint32_t index = m_RenderingInfo.device.FindMemoryIndex(std::vector<vk::MemoryRequirements>{requires}, vk::MemoryPropertyFlagBits::eDeviceLocal);
                vk::MemoryAllocateInfo ImageAllocateInfo(requires.size, index);
                m_TextureMemory = device.allocateMemory(ImageAllocateInfo);
                device.bindImageMemory(m_TextureImage, m_TextureMemory, 0);

                vk::BufferCreateInfo StagebufferInfo({}, textureInfo.dataSize, vk::BufferUsageFlagBits::eTransferSrc, vk::SharingMode::eExclusive);
                auto stageBuffer = device.createBuffer(StagebufferInfo);

                auto stageMemory = CreateMemoryAndBind(std::vector<vk::Buffer>{ stageBuffer }, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
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
            }

            //Set DesCriptorSetLayout
            {
                std::vector<vk::DescriptorSetLayoutBinding> bindings;
                bindings.reserve(3);
                vk::DescriptorSetLayoutBinding Binding_Mvp(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex);
                bindings.emplace_back(Binding_Mvp);

                vk::DescriptorSetLayoutCreateInfo CreateInfo({}, bindings);
                m_TriangleDescriptorSetLayout =  device.createDescriptorSetLayout(CreateInfo);
            }

            //Create descriptor pool
            {
                std::vector<vk::DescriptorPoolSize> poolSize{
                    vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 1)
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
            vk::VertexInputBindingDescription bindDes(0, sizeof(Vertex), vk::VertexInputRate::eVertex);

            vk::VertexInputAttributeDescription attrDes_position(0, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, Position));
            vk::VertexInputAttributeDescription attrDes_color(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, Color));
            vk::VertexInputAttributeDescription attrDes_TexCoord(2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, TexCoord));

            std::array<vk::VertexInputAttributeDescription, 3> attrDes{ attrDes_position, attrDes_color, attrDes_TexCoord };
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

        //device.freeDescriptorSets(m_DescriptorPool, m_TriangleDesCriptorSets);
        device.destroyDescriptorPool(m_DescriptorPool);
        device.destroyDescriptorSetLayout(m_TriangleDescriptorSetLayout);

        device.freeMemory(m_TextureMemory);
        device.destroyImage(m_TextureImage);

        device.unmapMemory(m_MVPMemory);
        device.freeMemory(m_MVPMemory);
        device.destroyBuffer(m_MVPUniformBuffer);

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

            drawCmd.bindVertexBuffers(0, m_VertexBuffer, {0});

            drawCmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_PipelineLayout, 0, m_TriangleDesCriptorSets, {});

            drawCmd.bindIndexBuffer(m_IndexBuffer, 0, vk::IndexType::eUint32);

            drawCmd.drawIndexed(Test_Indices.size(), 1, 0, 0, 0);
            //drawCmd.draw(Test_Vertices.size(), 1, 0, 0);

            drawCmd.endRenderPass();
            drawCmd.end();
        }

        auto& drawQueue = m_RenderingInfo.device.GetRenderingQueue().PresentQueue.value();

        //Updata MVP
        {
            static auto start = std::chrono::high_resolution_clock::now();
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration<float, std::chrono::seconds::period>(end - start).count();
            mvp.Model = glm::rotate(glm::mat4(1.0f), glm::radians(duration * 90.f), glm::vec3(0.0f, 0.0f, 1.0f));
            memcpy(m_MVPData, &mvp, sizeof(mvp));
        }

        std::array<vk::PipelineStageFlags, 1> pipelineStages{ vk::PipelineStageFlagBits::eColorAttachmentOutput };
        vk::SubmitInfo subInfo(m_GetImageSemaphores[currentFrame], pipelineStages, drawCmd, m_PresentSemaphores[currentFrame]);
        drawQueue.submit(subInfo, m_FrameFences[currentFrame]);

        vk::PresentInfoKHR presentInfo(m_PresentSemaphores[currentFrame], swapchain, nextImageIndex);
        auto result_present = drawQueue.presentKHR(presentInfo);

        currentFrame = (currentFrame + 1) % MaxFrameInFlight;
    }

} // namespace mortal
