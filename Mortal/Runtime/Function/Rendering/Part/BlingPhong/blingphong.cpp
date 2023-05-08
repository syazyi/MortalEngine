#include "blingphong.h"

namespace mortal
{
    BlingPhong::BlingPhong(RenderingSystemInfo& info) : RenderPartBase(info), m_UITool(info)
    {
        Init();
        m_UITool.InitUI(m_BlingPhongPass);
    }

    BlingPhong::~BlingPhong()
    {
        ClearUp();
    }

    void BlingPhong::Init()
    {
        //Load Model
        m_ModelInfo = LoadObjModel("../../Asset/Model/Plane.obj");
        //Load skybox
        m_SkyboxModel = LoadObjModel("../../Asset/Model/cube.obj");
        //Load Texture
        auto textureInfo = LoadTexture("NahidaClip.png");
        //Load Skybox
        std::array<TextureInfo, 6> skyboxTexture{
            LoadTexture("skybox/right.jpg"),
            LoadTexture("skybox/left.jpg"),
            LoadTexture("skybox/top.jpg"),
            LoadTexture("skybox/bottom.jpg"),
            LoadTexture("skybox/front.jpg"),
            LoadTexture("skybox/back.jpg")
        };

        //mvp
        auto extent2D = m_RenderingInfo.window.GetExtent2D();

        mvp.model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        mvp.view = m_RenderingInfo.m_Camera.GetView();
        mvp.proj = glm::perspective(glm::radians(45.f), (float)extent2D.width / (float)extent2D.height, 0.1f, 100.f);
        mvp.proj[1][1] *= -1;

        //skybox mvp
        skyboxMvp.model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        skyboxMvp.view = glm::mat4(glm::mat3(mvp.view));
        //skyboxMvp.view = mvp.view;
        skyboxMvp.proj = mvp.proj;

        auto& device = m_RenderingInfo.device.GetDevice();
        auto& command = m_RenderingInfo.command;
        auto& graphicQueue = m_RenderingInfo.device.GetRenderingQueue().GraphicQueue.value();
        auto& swapchain = m_RenderingInfo.swapchain;
        auto depthFormatInfo = m_RenderingInfo.device.FindSupportDepthFormat(std::vector<vk::Format>{ vk::Format::eD24UnormS8Uint, vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint },
            vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);

        //model size
        auto vertices_size = m_ModelInfo.vertices.size() * sizeof(m_ModelInfo.vertices[0]);
        auto indices_size = m_ModelInfo.indeices.size() * sizeof(m_ModelInfo.indeices[0]);

        //skybox size
        auto skybox_vertices_size = m_SkyboxModel.vertices.size() * sizeof(m_SkyboxModel.vertices[0]);
        auto skybox_indices_size = m_SkyboxModel.indeices.size() * sizeof(m_SkyboxModel.indeices[0]);
        //create vertex buffer index buffer and texture image include sampler and depth Image
        {
            //model vertex
            m_VertexBuffer = CreateBufferExclusive(vertices_size, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst);
            m_IndexBuffer = CreateBufferExclusive(indices_size, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst);
            m_VertexIndexMemory = CreateMemoryAndBind_Buffer(std::vector<vk::Buffer>{ m_VertexBuffer, m_IndexBuffer }, vk::MemoryPropertyFlagBits::eDeviceLocal);
            
            //skybox vertex
            m_SkyboxVertexBuffer = CreateBufferExclusive(skybox_vertices_size, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst);
            m_SkyboxIndexBuffer = CreateBufferExclusive(skybox_indices_size, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst);
            m_SkyboxVertexIndexMemory = CreateMemoryAndBind_Buffer(std::vector<vk::Buffer>{ m_SkyboxVertexBuffer, m_SkyboxIndexBuffer }, vk::MemoryPropertyFlagBits::eDeviceLocal);

            //model texture
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

            //skybox texture
            vk::ImageCreateInfo skyboxImageCreateInfo(vk::ImageCreateFlagBits::eCubeCompatible, vk::ImageType::e2D, vk::Format::eR8G8B8A8Srgb, vk::Extent3D{ static_cast<uint32_t>(skyboxTexture[0].texWidth), static_cast<uint32_t>(skyboxTexture[0].texHeight), 1},
                1, 6, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, vk::SharingMode::eExclusive);
            m_SkyboxTexture = device.createImage(skyboxImageCreateInfo);
            m_SkyboxMemory = CreateMemoryAndBind_Image(m_SkyboxTexture, vk::MemoryPropertyFlagBits::eDeviceLocal);
            vk::SamplerCreateInfo skyboxSamplerCreateInfo({}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear,
                vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat, 0.0f, VK_TRUE, 1.0f, VK_FALSE, vk::CompareOp::eNever,
                0.0f, 1.0f, vk::BorderColor::eFloatOpaqueBlack, VK_FALSE);
            m_SkyboxSampler = device.createSampler(skyboxSamplerCreateInfo);
            vk::ImageViewCreateInfo skyboxImageViewCreateInfo({}, m_SkyboxTexture, vk::ImageViewType::eCube, vk::Format::eR8G8B8A8Srgb,
                vk::ComponentMapping(vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA), 
                vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 6));
            m_SkyboxView = device.createImageView(skyboxImageViewCreateInfo);

            //depth image
            vk::ImageCreateInfo depthImageCreateInfo({}, vk::ImageType::e2D, depthFormatInfo.first, vk::Extent3D(extent2D, 1.0f), 1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, 
                vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::SharingMode::eExclusive);
            m_DepthImage = device.createImage(depthImageCreateInfo);
            m_DepthImageMemory = CreateMemoryAndBind_Image(m_DepthImage, vk::MemoryPropertyFlagBits::eDeviceLocal);
            vk::ImageViewCreateInfo depthImageViewCreateInfo({}, m_DepthImage, vk::ImageViewType::e2D, depthFormatInfo.first,
                vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1));
            m_DepthImageView = device.createImageView(depthImageViewCreateInfo);

            //model uniform buffer
            m_MvpBuffer = CreateBufferExclusive(sizeof(MVP), vk::BufferUsageFlagBits::eUniformBuffer);
            m_MvpMemory = CreateMemoryAndBind_Buffer(std::vector<vk::Buffer>{ m_MvpBuffer }, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);
            m_MvpData = device.mapMemory(m_MvpMemory, 0, sizeof(MVP));

            //skybox uniform buffre
            m_SkyboxMvpBuffer = CreateBufferExclusive(sizeof(MVP), vk::BufferUsageFlagBits::eUniformBuffer);
            m_SkyboxMvpMemory = CreateMemoryAndBind_Buffer(std::vector<vk::Buffer>{ m_SkyboxMvpBuffer }, vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);
            m_SkyboxMvpData = device.mapMemory(m_SkyboxMvpMemory, 0, sizeof(MVP));
        }
        //copy vertex, index, image info 
        {
            //model vertex index copy
            vk::Buffer vertexIndexStageBuffer = CreateBufferExclusive(vertices_size + indices_size, vk::BufferUsageFlagBits::eTransferSrc);
            vk::DeviceMemory vertexIndexStageMemory = CreateMemoryAndBind_Buffer(std::vector<vk::Buffer>{ vertexIndexStageBuffer }, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
            void* data = device.mapMemory(vertexIndexStageMemory, 0, vertices_size);
            memcpy(data, m_ModelInfo.vertices.data(), vertices_size);
            device.unmapMemory(vertexIndexStageMemory);

            data = device.mapMemory(vertexIndexStageMemory, vertices_size, indices_size);
            memcpy(data, m_ModelInfo.indeices.data(), indices_size);
            device.unmapMemory(vertexIndexStageMemory);

            //skybox vertex index copy
            vk::Buffer skybox_VertexIndexStageBuffer = CreateBufferExclusive(skybox_vertices_size + skybox_indices_size, vk::BufferUsageFlagBits::eTransferSrc);
            vk::DeviceMemory skybox_VertexIndexStageMemory = CreateMemoryAndBind_Buffer(std::vector<vk::Buffer>{ skybox_VertexIndexStageBuffer }, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
            data = device.mapMemory(skybox_VertexIndexStageMemory, 0, skybox_vertices_size);
            memcpy(data, m_SkyboxModel.vertices.data(), skybox_vertices_size);
            device.unmapMemory(skybox_VertexIndexStageMemory);

            data = device.mapMemory(skybox_VertexIndexStageMemory, skybox_vertices_size, skybox_indices_size);
            memcpy(data, m_SkyboxModel.indeices.data(), skybox_indices_size);
            device.unmapMemory(skybox_VertexIndexStageMemory);

            //model texture copy
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
            
            //skybox texture copy
            vk::Buffer skybox_TextureStageBuffer = CreateBufferExclusive(skyboxTexture[0].dataSize * 6, vk::BufferUsageFlagBits::eTransferSrc);
            vk::DeviceMemory skybox_TextureStageMemory = CreateMemoryAndBind_Buffer(std::vector<vk::Buffer>{ skybox_TextureStageBuffer }, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
            data = device.mapMemory(skybox_TextureStageMemory, 0, skyboxTexture[0].dataSize * 6);
            for (uint32_t i = 0; i < 6; i++) {
                memcpy(reinterpret_cast<void*>(reinterpret_cast<uint8_t*>(data) + i * skyboxTexture[i].dataSize), skyboxTexture[i].data, skyboxTexture[i].dataSize);
            }
            device.unmapMemory(skybox_TextureStageMemory);

            cmd = command.BeginSingleCommand();
            cmd.copyBuffer(skybox_VertexIndexStageBuffer, m_SkyboxVertexBuffer, { vk::BufferCopy(0, 0, skybox_vertices_size) });
            cmd.copyBuffer(skybox_VertexIndexStageBuffer, m_SkyboxIndexBuffer, { vk::BufferCopy(skybox_vertices_size, 0, skybox_indices_size) });

            vk::ImageMemoryBarrier skybox_UndefineToTransferDst(vk::AccessFlagBits::eNone, vk::AccessFlagBits::eTransferWrite,
                vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, {}, {},
                m_SkyboxTexture, vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 6));
            cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, { skybox_UndefineToTransferDst });

            std::vector<vk::BufferImageCopy> regions;

            for (int i = 0; i < 6; i++) {
                regions.push_back(vk::BufferImageCopy(i * skyboxTexture[0].dataSize, skyboxTexture[i].texWidth, skyboxTexture[i].texHeight,
                    vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, i, 1), vk::Offset3D(0, 0, 0), vk::Extent3D(skyboxTexture[i].texWidth, skyboxTexture[i].texHeight, 1.0f)));
            }
            cmd.copyBufferToImage(skybox_TextureStageBuffer, m_SkyboxTexture, vk::ImageLayout::eTransferDstOptimal, regions);

            vk::ImageMemoryBarrier skybox_TransferDstToShaderRead(vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead,
                vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, {}, {},
                m_SkyboxTexture, vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 6));
            cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, { skybox_TransferDstToShaderRead });

            command.EndSingleCommand(cmd, graphicQueue);

            //clear up stage info
            device.freeMemory(skybox_TextureStageMemory);
            device.destroyBuffer(skybox_TextureStageBuffer);
            device.freeMemory(skybox_VertexIndexStageMemory);
            device.destroyBuffer(skybox_VertexIndexStageBuffer);

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
                vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 2),
                vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, 2)
            };
            vk::DescriptorPoolCreateInfo poolCreateInfo({}, 2, poolSizes);
            m_DescriptorPool = device.createDescriptorPool(poolCreateInfo);

            vk::DescriptorSetAllocateInfo setAllocateInfo(m_DescriptorPool, m_MvpAndSamplerSetLayout);
            m_MvpAndSamplerSets = device.allocateDescriptorSets(setAllocateInfo);
            m_SkyboxDescriptorSets = device.allocateDescriptorSets(setAllocateInfo);

            vk::DescriptorBufferInfo MvpbufferInfo(m_MvpBuffer, 0, sizeof(MVP));
            vk::DescriptorImageInfo samplerImageInfo(m_TextureSampler, m_TextureImageView, vk::ImageLayout::eShaderReadOnlyOptimal);
            device.updateDescriptorSets({
                vk::WriteDescriptorSet(m_MvpAndSamplerSets[0], 0, 0, vk::DescriptorType::eUniformBuffer, {}, MvpbufferInfo),
                vk::WriteDescriptorSet(m_MvpAndSamplerSets[0], 1, 0, vk::DescriptorType::eCombinedImageSampler, samplerImageInfo)
            }, {});

            vk::DescriptorBufferInfo skybox_MvpbufferInfo(m_SkyboxMvpBuffer, 0, sizeof(MVP));
            vk::DescriptorImageInfo skybox_SamplerImageInfo(m_SkyboxSampler, m_SkyboxView, vk::ImageLayout::eShaderReadOnlyOptimal);
            device.updateDescriptorSets({
                vk::WriteDescriptorSet(m_SkyboxDescriptorSets[0], 0, 0, vk::DescriptorType::eUniformBuffer, {}, skybox_MvpbufferInfo),
                vk::WriteDescriptorSet(m_SkyboxDescriptorSets[0], 1, 0, vk::DescriptorType::eCombinedImageSampler, skybox_SamplerImageInfo)
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

            auto inputAssemblyState = vk::PipelineInputAssemblyStateCreateInfo({}, vk::PrimitiveTopology::eTriangleList, VK_FALSE);
            vk::Viewport viewprot(0, 0, extent2D.width, extent2D.height, 0.0f, 1.0f);
            vk::Rect2D scissor({ 0, 0 }, extent2D);
            auto viewportState = vk::PipelineViewportStateCreateInfo({}, viewprot, scissor);
            auto rasterizationState = vk::PipelineRasterizationStateCreateInfo({}, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise,
                VK_FALSE, {}, {}, {}, 1.0f);
            auto multisampleState = vk::PipelineMultisampleStateCreateInfo({}, vk::SampleCountFlagBits::e1, VK_FALSE);
            auto depthStencilState = vk::PipelineDepthStencilStateCreateInfo({}, VK_TRUE, VK_TRUE, vk::CompareOp::eLess, VK_FALSE, VK_FALSE);

            vk::PipelineColorBlendAttachmentState state(VK_FALSE, vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eOneMinusSrcAlpha, vk::BlendOp::eAdd,
                vk::BlendFactor::eZero, vk::BlendFactor::eOne, vk::BlendOp::eAdd,
                vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
            auto colorBlendState = vk::PipelineColorBlendStateCreateInfo({}, VK_FALSE, vk::LogicOp::eCopy, state);

            std::vector<vk::DynamicState> dynamicStates{ vk::DynamicState::eViewport, vk::DynamicState::eScissor };
            vk::PipelineDynamicStateCreateInfo dynamicState({}, dynamicStates);

            auto vertShaderModule = CreateShaderModule("BlingPhong/BlingPhong_vert");
            auto fragShaderModule = CreateShaderModule("BlingPhong/BlingPhong_frag");
            vk::PipelineShaderStageCreateInfo vertshaderStage({}, vk::ShaderStageFlagBits::eVertex, vertShaderModule, "main");
            vk::PipelineShaderStageCreateInfo fragshaderStage({}, vk::ShaderStageFlagBits::eFragment, fragShaderModule, "main");
            std::vector<vk::PipelineShaderStageCreateInfo> shaderStages{ vertshaderStage, fragshaderStage };
            
            vk::PushConstantRange materialRange(vk::ShaderStageFlagBits::eFragment, 0, sizeof(BlingPhongMaterial));
            vk::PipelineLayoutCreateInfo layoutCreateInfo({}, m_MvpAndSamplerSetLayout, materialRange);
            m_BlingPhongPipelineLayout = device.createPipelineLayout(layoutCreateInfo);
            vk::GraphicsPipelineCreateInfo blingphongPipelineInfo({}, shaderStages, &vertexInput, &inputAssemblyState, nullptr, &viewportState, &rasterizationState,
                &multisampleState, &depthStencilState, &colorBlendState, &dynamicState, m_BlingPhongPipelineLayout, m_BlingPhongPass, 0, nullptr, -1);
            m_BlingPhongPipeline = device.createGraphicsPipeline({}, blingphongPipelineInfo).value;

            //skybox pipeline
            auto skybox_VertShaderModule = CreateShaderModule("BlingPhong/Skybox_vert");
            auto skybox_FragShaderModule = CreateShaderModule("BlingPhong/Skybox_frag");
            vk::PipelineShaderStageCreateInfo skybox_VertshaderStage({}, vk::ShaderStageFlagBits::eVertex, skybox_VertShaderModule, "main");
            vk::PipelineShaderStageCreateInfo skybox_FragshaderStage({}, vk::ShaderStageFlagBits::eFragment, skybox_FragShaderModule, "main");

            vk::PipelineLayoutCreateInfo skybox_LayoutCreateInfo({}, m_MvpAndSamplerSetLayout);
            m_SkyboxPipelineLayout = device.createPipelineLayout(skybox_LayoutCreateInfo);
            std::vector<vk::PipelineShaderStageCreateInfo> skybox_ShaderStages{ skybox_VertshaderStage, skybox_FragshaderStage };
            rasterizationState.setCullMode(vk::CullModeFlagBits::eFront);
            depthStencilState.setDepthTestEnable(VK_FALSE);
            depthStencilState.setDepthWriteEnable(VK_FALSE);

            vk::GraphicsPipelineCreateInfo skyboxPipelineInfo({}, skybox_ShaderStages, & vertexInput, & inputAssemblyState, nullptr, & viewportState, & rasterizationState,
                & multisampleState, & depthStencilState, & colorBlendState, & dynamicState, m_SkyboxPipelineLayout, m_BlingPhongPass, 0, nullptr, -1);
            m_SkyboxPipeline = device.createGraphicsPipeline({}, skyboxPipelineInfo).value;

            device.destroyShaderModule(skybox_VertShaderModule);
            device.destroyShaderModule(skybox_FragShaderModule);

            device.destroyShaderModule(vertShaderModule);
            device.destroyShaderModule(fragShaderModule);
        }
    }
    
    void BlingPhong::ClearUp()
    {
        auto& device = m_RenderingInfo.device.GetDevice();

        device.waitIdle();

        m_UITool.ClearUpUI();

        device.destroyPipeline(m_SkyboxPipeline);
        device.destroyPipelineLayout(m_SkyboxPipelineLayout);

        device.destroyPipeline(m_BlingPhongPipeline);
        device.destroyPipelineLayout(m_BlingPhongPipelineLayout);
        for (auto& framebuffer : m_Framebuffers) {
            device.destroyFramebuffer(framebuffer);
        }

        device.destroyRenderPass(m_BlingPhongPass);

        device.destroyDescriptorSetLayout(m_MvpAndSamplerSetLayout);
        device.destroyDescriptorPool(m_DescriptorPool);

        device.unmapMemory(m_SkyboxMvpMemory);
        device.freeMemory(m_SkyboxMvpMemory);
        device.destroyBuffer(m_SkyboxMvpBuffer);

        device.unmapMemory(m_MvpMemory);
        device.freeMemory(m_MvpMemory);
        device.destroyBuffer(m_MvpBuffer);

        device.destroyImageView(m_DepthImageView);
        device.freeMemory(m_DepthImageMemory);
        device.destroyImage(m_DepthImage);

        device.destroySampler(m_SkyboxSampler);
        device.destroyImageView(m_SkyboxView);
        device.freeMemory(m_SkyboxMemory);
        device.destroyImage(m_SkyboxTexture);

        device.destroySampler(m_TextureSampler);
        device.destroyImageView(m_TextureImageView);
        device.freeMemory(m_TextureImageMemory);
        device.destroyImage(m_TextureImage);

        device.freeMemory(m_SkyboxVertexIndexMemory);
        device.destroyBuffer(m_SkyboxIndexBuffer);
        device.destroyBuffer(m_SkyboxVertexBuffer);

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

            mvp.lightPos = glm::mat3(glm::rotate(glm::mat4(1.0f), glm::radians(duration * 90.f), glm::vec3(0.0f, 0.0f, 1.0f))) * glm::vec3(3.0f, 3.0f, 3.0f);
            //mvp.lightPos = glm::vec3(3.0f, 3.0f, 3.0f);
            mvp.model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            mvp.view = m_RenderingInfo.m_Camera.GetView();
            mvp.normalMat = glm::transpose(glm::inverse(mvp.view * mvp.model));
            memcpy(m_MvpData, &mvp, sizeof(mvp));

            skyboxMvp.view = glm::mat4(glm::mat3(mvp.view));
            memcpy(m_SkyboxMvpData, &skyboxMvp, sizeof(skyboxMvp));
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

        //skybox
        drawCmd.bindPipeline(vk::PipelineBindPoint::eGraphics, m_SkyboxPipeline);
        drawCmd.bindVertexBuffers(0, { m_SkyboxVertexBuffer }, {0});
        drawCmd.bindIndexBuffer(m_SkyboxIndexBuffer, 0, vk::IndexType::eUint32);
        drawCmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_SkyboxPipelineLayout, 0, m_SkyboxDescriptorSets, {});
        drawCmd.drawIndexed(m_SkyboxModel.indeices.size(), 1, 0, 0, 0);

        //bling phong
        drawCmd.bindPipeline(vk::PipelineBindPoint::eGraphics, m_BlingPhongPipeline);
        drawCmd.bindVertexBuffers(0, { m_VertexBuffer }, {0});
        drawCmd.bindIndexBuffer(m_IndexBuffer, 0, vk::IndexType::eUint32);

        drawCmd.pushConstants<BlingPhongMaterial>(m_BlingPhongPipelineLayout, vk::ShaderStageFlagBits::eFragment, 0u, materialInfo);
        drawCmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_BlingPhongPipelineLayout, 0, m_MvpAndSamplerSets, {});
        drawCmd.drawIndexed(m_ModelInfo.indeices.size(), 1, 0, 0, 0);

        m_UITool.Draw(drawCmd, [&material = materialInfo]() {
            ImGui::ColorEdit3("Light Color", &material.LightColor[0]);
            ImGui::InputInt("Cos q value:", &material.q);
            ImGui::ColorEdit3("Ka", &material.Ka[0]);
            ImGui::InputFloat("Attenuation constant:", &material.constant);
            ImGui::ColorEdit3("kd", &material.Kd[0]);
            ImGui::InputFloat("Attenuation linear:", &material.linear);
            ImGui::ColorEdit3("ks", &material.Ks[0]);
            ImGui::InputFloat("Attenuation quadratic:", &material.quadratic);
        });

        drawCmd.endRenderPass();
        drawCmd.end();
    }
} // namespace mortal
