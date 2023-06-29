#include "Particles.h"
#include <random>
namespace mortal
{
    ParticlesPart::ParticlesPart(VulkanContext& renderInfo) : RenderPartBase(renderInfo)
    {
        Init();
    }

    ParticlesPart::~ParticlesPart()
    {
        ClearUp();
    }

    void ParticlesPart::Init()
    {
        auto& device = m_RenderingInfo.device.GetDevice();
        auto extent2D = m_RenderingInfo.window.GetExtent2D();
        auto& computeIndex = m_RenderingInfo.device.GetRenderingQueue().ComputeQueueFamilyIndex.value();
        auto& computeQueue = m_RenderingInfo.device.GetRenderingQueue().ComputeQueue.value();
        auto& graphicIndex = m_RenderingInfo.device.GetRenderingQueue().GraphicQueueFamilyIndex.value();
        auto& graphicQueue = m_RenderingInfo.device.GetRenderingQueue().GraphicQueue.value();
        auto depthFormatInfo = m_RenderingInfo.device.FindSupportDepthFormat(
            std::vector<vk::Format>{ vk::Format::eD24UnormS8Uint, vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint }, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment
        );
        //compute prepare
        {
            auto particles = PrepareParticles();
            auto particles_size = particles.size() * sizeof(ParticlesInfo);
            vk::BufferCreateInfo particlesBufferCI({}, particles_size, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
                vk::SharingMode::eExclusive);
            m_CurrentParticlesBuffer = device.createBuffer(particlesBufferCI);
            m_CurrentParticlesBufferMemory = CreateMemoryAndBind_Buffer(std::vector<vk::Buffer>{ m_CurrentParticlesBuffer }, vk::MemoryPropertyFlagBits::eDeviceLocal);

            //ToDo: stage buffer and copy , use barrier if graphic queue isnot same with compute queue. you need an commandpool allocate compute command buffer.
            //Stage buffer and store data
            auto stageBuffer = CreateBufferExclusive(particles_size, vk::BufferUsageFlagBits::eTransferSrc);
            auto stageBufferMemory = CreateMemoryAndBind_Buffer(std::vector<vk::Buffer>{ stageBuffer }, 
                vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);
            void* data = device.mapMemory(stageBufferMemory, 0, particles_size);
            memcpy(data, particles.data(), particles_size);
            device.unmapMemory(stageBufferMemory);

            m_CurrentParticlesUBO = PrepareUniform<computeUBO>();
            m_CurrentParticlesUBOData.particleCount = ParticlesCount;
            memcpy(m_CurrentParticlesUBO.mapped, &m_CurrentParticlesUBOData, sizeof(computeUBO));
            //Prepare Command 
            vk::CommandPoolCreateInfo computeCommandPoolCI(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, computeIndex);
            m_ComputeCommandPool = device.createCommandPool(computeCommandPoolCI);
            m_ComputeCommands = device.allocateCommandBuffers(vk::CommandBufferAllocateInfo(m_ComputeCommandPool, vk::CommandBufferLevel::ePrimary, MaxFrameInFlight));

            //copy buffer
            auto singleComputeCmd = m_ComputeCommands[0];
            singleComputeCmd.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
            
            singleComputeCmd.copyBuffer(stageBuffer, m_CurrentParticlesBuffer, {
                vk::BufferCopy(0, 0, particles_size)
                });
            if (graphicIndex != computeIndex) {
                //vk::BufferMemoryBarrier CopyBufferToComputeBarrier(vk::AccessFlagBits::eMemoryWrite, vk::AccessFlagBits::eNone, computeIndex, computeIndex, m_CurrentParticlesBuffer, 0, particles_size);
                //singleComputeCmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eNone, {}, {}, CopyBufferToComputeBarrier, {});
            }

            singleComputeCmd.end();
            vk::SubmitInfo singleSubMitI({}, {}, singleComputeCmd, {});
            computeQueue.submit(singleSubMitI);
            computeQueue.waitIdle();
            device.freeMemory(stageBufferMemory);
            device.destroyBuffer(stageBuffer);

            //Descriptor
            std::vector<vk::DescriptorPoolSize> poolSizes{
                vk::DescriptorPoolSize(vk::DescriptorType::eStorageBuffer, 1),
                vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 2),
                vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, 1)
            };
            vk::DescriptorPoolCreateInfo decriptorPoolCI({}, 2, poolSizes);
            m_ParticlesDescriptorPool = device.createDescriptorPool(decriptorPoolCI);

            //allocate particles
            std::vector<vk::DescriptorSetLayoutBinding> DescriptorSetLayoutBinding{
                vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), 
                vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eCompute)
            };
            vk::DescriptorSetLayoutCreateInfo ParticlesDSLCI({}, DescriptorSetLayoutBinding);
            m_ParticlesDescriptorSetLayout = device.createDescriptorSetLayout(ParticlesDSLCI);
            vk::DescriptorSetAllocateInfo ParticlesDSA(m_ParticlesDescriptorPool, m_ParticlesDescriptorSetLayout);
            m_ParticlesDescriptorSets = device.allocateDescriptorSets(ParticlesDSA);

            //update descriptor
            vk::DescriptorBufferInfo particlesStorageDBI(m_CurrentParticlesBuffer, 0, particles_size);
            vk::DescriptorBufferInfo particlesUniformDBI(m_CurrentParticlesUBO.uniformBuffer, 0, sizeof(computeUBO));
            device.updateDescriptorSets({
                    vk::WriteDescriptorSet(m_ParticlesDescriptorSets[0], 0, 0, vk::DescriptorType::eStorageBuffer, {}, particlesStorageDBI), 
                    vk::WriteDescriptorSet(m_ParticlesDescriptorSets[0], 1, 0, vk::DescriptorType::eUniformBuffer, {}, particlesUniformDBI)
                }, {});

            //pipelinelayout and pipeline Set
            vk::PipelineLayoutCreateInfo ParticlesPLCI({}, m_ParticlesDescriptorSetLayout);
            m_ParticlesPipelineLayout = device.createPipelineLayout(ParticlesPLCI);
            auto ComputeShaderModule = CreateShaderModule("Particles/Particles_comp");
            vk::ComputePipelineCreateInfo computerPCI({}, vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eCompute, ComputeShaderModule, "main"), m_ParticlesPipelineLayout);
            m_ParticlesPipeline = device.createComputePipeline(nullptr, computerPCI).value;
            device.destroyShaderModule(ComputeShaderModule);
        }

        vk::SemaphoreCreateInfo semaphoreCI;
        m_ComputeToGraphic = device.createSemaphore(semaphoreCI);

        m_GraphicToCompute = device.createSemaphore(semaphoreCI);
        std::array<vk::Semaphore, 1> initSemaphore{ m_GraphicToCompute };
        graphicQueue.submit(vk::SubmitInfo({}, {}, {}, initSemaphore));
        graphicQueue.waitIdle();

        //graphic prepare
        {
            m_SceneUBOData.proj = glm::perspective(glm::radians(45.f), (float)extent2D.width / (float)extent2D.height, 0.1f, 100.f);
            m_SceneUBOData.proj[1][1] *= -1;
            m_SceneUBO = PrepareUniform<SceneUBO>();

            //image and sampler
            auto textureInfo = LoadTexture("particle_fire.png");
            vk::ImageCreateInfo ColorImageCI({}, vk::ImageType::e2D, vk::Format::eR8G8B8A8Srgb, vk::Extent3D(textureInfo.texWidth, textureInfo.texHeight, 1), 1, 1,
                vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, vk::SharingMode::eExclusive);
            m_ParticlesColor = device.createImage(ColorImageCI);
            m_ParticlesColorMemory = CreateMemoryAndBind_Image(m_ParticlesColor, vk::MemoryPropertyFlagBits::eDeviceLocal);
            vk::ImageViewCreateInfo textureImageViewCreateInfo({}, m_ParticlesColor, vk::ImageViewType::e2D, vk::Format::eR8G8B8A8Srgb,
                vk::ComponentMapping(vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA),
                vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
            m_ParticlesColorView = device.createImageView(textureImageViewCreateInfo);
            vk::SamplerCreateInfo textureSamplerCreateInfo({}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear,
                vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat, 0.0f, VK_TRUE, 1.0f, VK_FALSE, vk::CompareOp::eAlways,
                0.0f, 1.0f, vk::BorderColor::eFloatOpaqueBlack, VK_FALSE);
            m_ParticlesColorSampler = device.createSampler(textureSamplerCreateInfo);

            //copy image
            auto stageBuffer = CreateBufferExclusive(textureInfo.dataSize, vk::BufferUsageFlagBits::eTransferSrc);
            auto stageBufferMemory = CreateMemoryAndBind_Buffer(std::vector<vk::Buffer>{ stageBuffer },
                vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);
            void* data = device.mapMemory(stageBufferMemory, 0, textureInfo.dataSize);
            memcpy(data, textureInfo.data, textureInfo.dataSize);
            device.unmapMemory(stageBufferMemory);

            auto singleCmd = m_RenderingInfo.command.BeginSingleCommand();
            vk::ImageMemoryBarrier TransitionBeginBarrier(vk::AccessFlagBits::eNone, vk::AccessFlagBits::eTransferWrite, 
                vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, m_ParticlesColor, 
                vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
            singleCmd.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, TransitionBeginBarrier);

            singleCmd.copyBufferToImage(stageBuffer, m_ParticlesColor, vk::ImageLayout::eTransferDstOptimal, { vk::BufferImageCopy(0, textureInfo.texWidth, textureInfo.texHeight, vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1),
                vk::Offset3D(0, 0, 0), vk::Extent3D(textureInfo.texWidth, textureInfo.texHeight, 1)) });

            vk::ImageMemoryBarrier TransitionEndBarrier(vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead,
                vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED, m_ParticlesColor,
                vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
            singleCmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, TransitionEndBarrier);
            m_RenderingInfo.command.EndSingleCommand(singleCmd, graphicQueue);

            device.freeMemory(stageBufferMemory);
            device.destroyBuffer(stageBuffer);

            vk::ImageCreateInfo sceneDepthImageCI({}, vk::ImageType::e2D, depthFormatInfo.first, vk::Extent3D(extent2D.width, extent2D.height, 1), 1, 1, vk::SampleCountFlagBits::e1,
                vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::SharingMode::eExclusive);
            m_DepthImage = device.createImage(sceneDepthImageCI);
            m_DepthImageMemory = CreateMemoryAndBind_Image(m_DepthImage, vk::MemoryPropertyFlagBits::eDeviceLocal);
            vk::ImageViewCreateInfo sceneImageViewCreateInfo({}, m_DepthImage, vk::ImageViewType::e2D, depthFormatInfo.first,
                vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1));
            m_DepthImageView = device.createImageView(sceneImageViewCreateInfo);


            //allocate graphics descriptor
            std::vector<vk::DescriptorSetLayoutBinding> graphicDescriptorSetLayoutBinding{
                vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex),
                vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment)
            };
            vk::DescriptorSetLayoutCreateInfo DSLCI({}, graphicDescriptorSetLayoutBinding);
            m_DescriptorSetLayout = device.createDescriptorSetLayout(DSLCI);
            vk::DescriptorSetAllocateInfo DSA(m_ParticlesDescriptorPool, m_DescriptorSetLayout);
            m_DescriptorSets = device.allocateDescriptorSets(DSA);

            vk::DescriptorBufferInfo UniformDBI(m_SceneUBO.uniformBuffer, 0, sizeof(SceneUBO));
            vk::DescriptorImageInfo ImageDII(m_ParticlesColorSampler, m_ParticlesColorView, vk::ImageLayout::eShaderReadOnlyOptimal);
            vk::WriteDescriptorSet UniformWDS(m_DescriptorSets[0], 0, 0, vk::DescriptorType::eUniformBuffer, {}, UniformDBI);
            vk::WriteDescriptorSet SamplerWDS(m_DescriptorSets[0], 1, 0, vk::DescriptorType::eCombinedImageSampler, ImageDII);
            device.updateDescriptorSets({ UniformWDS , SamplerWDS }, {});

            //render pass and Framebuffer
            {
                vk::ImageLayout beUsedDepthLayout = vk::ImageLayout::eDepthAttachmentOptimal;
                if (depthFormatInfo.second) {
                    beUsedDepthLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
                }
                vk::AttachmentDescription attachDes_Color({}, m_RenderingInfo.swapchain.GetSurfaceDetail().SurfaceFormats.format, vk::SampleCountFlagBits::e1,
                    vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
                    vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR);
                vk::AttachmentDescription attachDes_Depth = vk::AttachmentDescription({}, depthFormatInfo.first, vk::SampleCountFlagBits::e1,
                    vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eDontCare, vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
                    vk::ImageLayout::eUndefined, beUsedDepthLayout);
                std::array<vk::AttachmentDescription, 2> attachDes{
                    attachDes_Color, attachDes_Depth
                };
                vk::AttachmentReference attachRef_Color(0, vk::ImageLayout::eColorAttachmentOptimal);
                vk::AttachmentReference attachRef_Depth(1, beUsedDepthLayout);
                vk::SubpassDescription sceneSubpass({}, vk::PipelineBindPoint::eGraphics, {}, attachRef_Color, {}, &attachRef_Depth);
                vk::SubpassDependency subPassDependency(VK_SUBPASS_EXTERNAL, 0, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput,
                    vk::AccessFlagBits::eNone, vk::AccessFlagBits::eColorAttachmentWrite);
                vk::RenderPassCreateInfo sceneRenderPassCI({}, attachDes, sceneSubpass, subPassDependency);
                m_RenderPass = device.createRenderPass(sceneRenderPassCI);

                auto presentImageViews = m_RenderingInfo.swapchain.GetSwapChainImageViews();
                for (int i = 0; i < presentImageViews.size(); i++) {
                    std::array<vk::ImageView, 2> attachments{
                        presentImageViews[i],
                        m_DepthImageView
                    };
                    vk::FramebufferCreateInfo frameCI({}, m_RenderPass, attachments, extent2D.width, extent2D.height, 1);
                    m_Framebuffers.push_back(device.createFramebuffer(frameCI));
                }
            }

            //pipeline prepare
            vk::PipelineLayoutCreateInfo scenePipelineLayoutCI({}, m_DescriptorSetLayout);
            m_GraphicPipelineLayout = device.createPipelineLayout(scenePipelineLayoutCI);

            vk::VertexInputBindingDescription binddes(0, sizeof(ParticlesInfo));
            vk::VertexInputAttributeDescription attrDes_Pos(0, 0, vk::Format::eR32G32B32Sfloat, 0);
            std::vector<vk::VertexInputAttributeDescription> attrDes{
                attrDes_Pos
            };
            auto vertexInput = vk::PipelineVertexInputStateCreateInfo({}, binddes, attrDes);

            auto inputAssemblyState = vk::PipelineInputAssemblyStateCreateInfo({}, vk::PrimitiveTopology::ePointList, VK_FALSE);
            vk::Viewport viewprot(0, 0, extent2D.width, extent2D.height, 0.0f, 1.0f);
            vk::Rect2D scissor({ 0, 0 }, { extent2D.width, extent2D.height });
            auto viewportState = vk::PipelineViewportStateCreateInfo({}, viewprot, scissor);
            auto rasterizationState = vk::PipelineRasterizationStateCreateInfo({}, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone, vk::FrontFace::eCounterClockwise,
                VK_FALSE, {}, {}, {}, 1.0f);
            auto multisampleState = vk::PipelineMultisampleStateCreateInfo({}, vk::SampleCountFlagBits::e1, VK_FALSE);
            auto depthStencilState = vk::PipelineDepthStencilStateCreateInfo({}, 0, 0, vk::CompareOp::eLessOrEqual, VK_FALSE, VK_FALSE);

            vk::PipelineColorBlendAttachmentState state(VK_TRUE, vk::BlendFactor::eOne, vk::BlendFactor::eOne, vk::BlendOp::eAdd,
                vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eDstAlpha, vk::BlendOp::eAdd,
                vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
            auto colorBlendState = vk::PipelineColorBlendStateCreateInfo({}, VK_FALSE, vk::LogicOp::eCopy, state);

            std::vector<vk::DynamicState> dynamicStates{ vk::DynamicState::eViewport, vk::DynamicState::eScissor };
            vk::PipelineDynamicStateCreateInfo dynamicState({}, dynamicStates);

            auto sceneVertShaderModule = CreateShaderModule("Particles/Particels_vert");
            auto sceneFragShaderModule = CreateShaderModule("Particles/Particels_frag");
            std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages{
                vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eVertex, sceneVertShaderModule, "main"),
                vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eFragment, sceneFragShaderModule, "main")
            };

            vk::GraphicsPipelineCreateInfo ScenePipelineCI({}, shaderStages, &vertexInput, &inputAssemblyState, nullptr, &viewportState, &rasterizationState,
                &multisampleState, &depthStencilState, &colorBlendState, &dynamicState, m_GraphicPipelineLayout, m_RenderPass, 0, nullptr, -1);
            m_GraphicPipeline = device.createGraphicsPipeline({}, ScenePipelineCI).value;

            device.destroyShaderModule(sceneFragShaderModule);
            device.destroyShaderModule(sceneVertShaderModule);
        }
    }

    void ParticlesPart::ClearUp()
    {
        auto& device = m_RenderingInfo.device.GetDevice();
        device.waitIdle();
        //clear up
        device.destroyDescriptorSetLayout(m_DescriptorSetLayout);
        device.destroyPipelineLayout(m_GraphicPipelineLayout);
        device.destroyPipeline(m_GraphicPipeline);
        for (auto& framebuffer : m_Framebuffers) {
            device.destroyFramebuffer(framebuffer);
        }
        device.destroyRenderPass(m_RenderPass);

        ClearUpPrepareUniform(m_SceneUBO);
        device.destroySampler(m_ParticlesColorSampler);
        device.destroyImageView(m_ParticlesColorView);
        device.freeMemory(m_ParticlesColorMemory);
        device.destroyImage(m_ParticlesColor);

        device.destroyImageView(m_DepthImageView);
        device.freeMemory(m_DepthImageMemory);
        device.destroyImage(m_DepthImage);

        device.destroySemaphore(m_GraphicToCompute);
        device.destroySemaphore(m_ComputeToGraphic);

        device.destroyPipelineLayout(m_ParticlesPipelineLayout);
        device.destroyPipeline(m_ParticlesPipeline);
        device.destroyCommandPool(m_ComputeCommandPool);
        device.destroyDescriptorSetLayout(m_ParticlesDescriptorSetLayout);
        device.destroyDescriptorPool(m_ParticlesDescriptorPool);

        ClearUpPrepareUniform(m_CurrentParticlesUBO);
        device.freeMemory(m_CurrentParticlesBufferMemory);
        device.destroyBuffer(m_CurrentParticlesBuffer);
    }

    void ParticlesPart::Draw()
    {
        PrepareFrame();
        auto currentFrame = m_RenderingInfo.CurrentFrame;
        //compute command
        //TODO: you need to set barrier.
        {
            auto Cmd = m_ComputeCommands[currentFrame];
            auto& computeQueue = m_RenderingInfo.device.GetRenderingQueue().ComputeQueue.value();
            Cmd.reset();
            Cmd.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
            Cmd.bindPipeline(vk::PipelineBindPoint::eCompute, m_ParticlesPipeline);
            Cmd.bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_ParticlesPipelineLayout, 0, m_ParticlesDescriptorSets, {});
            Cmd.dispatch(ParticlesCount / ParticelsPatchCount, 1, 1);
            Cmd.end();

            std::array<vk::Semaphore, 1> computeWaitSemaphores{ m_GraphicToCompute };
            std::array<vk::Semaphore, 1> computeSignalSemaphores{ m_ComputeToGraphic };
            std::array<vk::PipelineStageFlags, 1> WaitStages{ vk::PipelineStageFlagBits::eComputeShader };

            vk::SubmitInfo computerSubInfo(computeWaitSemaphores, WaitStages, Cmd, computeSignalSemaphores);
            computeQueue.submit(computerSubInfo);
        }

        {
            {
                m_SceneUBOData.model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                m_SceneUBOData.view = m_RenderingInfo.m_Camera.GetView();
                memcpy(m_SceneUBO.mapped, &m_SceneUBOData, sizeof(SceneUBO));
            }
            auto Cmd = m_RenderingInfo.command.GetCommandBuffers()[currentFrame];
            auto& graphicQueue = m_RenderingInfo.device.GetRenderingQueue().GraphicQueue.value();
            auto extent2D = m_RenderingInfo.window.GetExtent2D();
            Cmd.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit));
            std::array<vk::ClearValue, 2> clearVaules{
                vk::ClearColorValue(std::array<float, 4>{0.025f, 0.025f, 0.025f, 1.0f}),
                vk::ClearDepthStencilValue(1.0f, 1)
            };
            Cmd.beginRenderPass(vk::RenderPassBeginInfo(m_RenderPass, m_Framebuffers[m_RenderingInfo.nextImageIndex], vk::Rect2D({}, extent2D), clearVaules),
                vk::SubpassContents::eInline);
            Cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, m_GraphicPipeline);
            Cmd.setViewport(0, vk::Viewport(0.0f, 0.0f, extent2D.width, extent2D.height, 0.0f, 1.0f));
            Cmd.setScissor(0, vk::Rect2D({}, extent2D));
            Cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_GraphicPipelineLayout, 0, m_DescriptorSets, {});
            Cmd.bindVertexBuffers(0, m_CurrentParticlesBuffer, {0});
            Cmd.draw(ParticlesCount, 1, 0, 0);
            Cmd.endRenderPass();
            Cmd.end();

            std::array<vk::Semaphore, 2> graphicWaits{ m_ComputeToGraphic, m_RenderingInfo.SemphoreInfo->m_GetImageSemaphores[currentFrame] };
            std::array<vk::Semaphore, 2> graphicSignalSemaphores{ m_GraphicToCompute,  m_RenderingInfo.SemphoreInfo->m_PresentSemaphores[currentFrame]};
            std::array<vk::PipelineStageFlags, 2> graphicWaitStageMask{ vk::PipelineStageFlagBits::eVertexInput, vk::PipelineStageFlagBits::eColorAttachmentOutput };
            vk::SubmitInfo graphicSubmitInfo(graphicWaits, graphicWaitStageMask, Cmd, graphicSignalSemaphores);
            graphicQueue.submit(graphicSubmitInfo, m_RenderingInfo.SemphoreInfo->m_FrameFences[currentFrame]);
        }

        auto& presentQueue = m_RenderingInfo.device.GetRenderingQueue().PresentQueue.value();
        vk::PresentInfoKHR presentInfo(m_RenderingInfo.SemphoreInfo->m_PresentSemaphores[currentFrame], m_RenderingInfo.swapchain.GetSwapChain(), m_RenderingInfo.nextImageIndex);
        auto result_present = presentQueue.presentKHR(presentInfo);
    }

    std::vector<ParticlesInfo> ParticlesPart::PrepareParticles()
    {
        std::default_random_engine rndEngine((unsigned)time(nullptr));
        std::uniform_real_distribution<float> rndDist(-1.0f, 1.0f);

        // Initial particle positions
        std::vector<ParticlesInfo> particleBuffer(ParticlesCount);
        for (auto& particle : particleBuffer) {
            particle.Position = glm::vec3(rndDist(rndEngine), rndDist(rndEngine), rndDist(rndEngine));
            particle.Velocity = glm::vec3(0.0f);
            //particle.gradientPos.x = particle.pos.x / 2.0f;
        }
        return particleBuffer;
    }

} // namespace mortal
