#include "rendering_part_base.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

namespace mortal
{
    static VmaAllocator m_VmaAllocator;


	RenderPartBase::RenderPartBase(VulkanContext& info) : m_RenderingInfo(info)
	{
        VmaAllocatorCreateInfo allocatorCI{};
        allocatorCI.vulkanApiVersion = VK_API_VERSION_1_3;
        allocatorCI.physicalDevice = m_RenderingInfo.device.GetPhysicalDevice();
        allocatorCI.device = m_RenderingInfo.device.GetDevice();
        allocatorCI.instance = m_RenderingInfo.device.GetInstanceRef();
        vmaCreateAllocator(&allocatorCI, &m_VmaAllocator);
    }

    void RenderPartBase::PrepareFrame()
    {
        auto& device = m_RenderingInfo.device.GetDevice();
        auto& currentFrame = m_RenderingInfo.CurrentFrame;
        auto result_waitFence = device.waitForFences(m_RenderingInfo.SemphoreInfo->m_FrameFences[currentFrame], VK_TRUE, UINT64_MAX);

        auto& swapchain = m_RenderingInfo.swapchain.GetSwapChain();
        auto result_nextImageIndex = device.acquireNextImageKHR(swapchain, UINT64_MAX, m_RenderingInfo.SemphoreInfo->m_GetImageSemaphores[currentFrame]);
        m_RenderingInfo.nextImageIndex = result_nextImageIndex.value;

        device.resetFences(m_RenderingInfo.SemphoreInfo->m_FrameFences[currentFrame]);

        auto& drawCmd = m_RenderingInfo.command.GetCommandBuffers()[currentFrame];
        drawCmd.reset();
    }

    void RenderPartBase::SubmitQueueSync()
    {
        auto& drawCmd = m_RenderingInfo.command.GetCommandBuffers()[m_RenderingInfo.CurrentFrame];
        auto& drawQueue = m_RenderingInfo.device.GetRenderingQueue().PresentQueue.value();

        std::array<vk::PipelineStageFlags, 1> pipelineStages{ vk::PipelineStageFlagBits::eColorAttachmentOutput };
        vk::SubmitInfo subInfo(m_RenderingInfo.SemphoreInfo->m_GetImageSemaphores[m_RenderingInfo.CurrentFrame], pipelineStages, drawCmd, m_RenderingInfo.SemphoreInfo->m_PresentSemaphores[m_RenderingInfo.CurrentFrame]);
        drawQueue.submit(subInfo, m_RenderingInfo.SemphoreInfo->m_FrameFences[m_RenderingInfo.CurrentFrame]);
                
        vk::PresentInfoKHR presentInfo(m_RenderingInfo.SemphoreInfo->m_PresentSemaphores[m_RenderingInfo.CurrentFrame], m_RenderingInfo.swapchain.GetSwapChain(), m_RenderingInfo.nextImageIndex);
        auto result_present = drawQueue.presentKHR(presentInfo);
    }

    glm::mat4 RenderPartBase::GetBlendCorrectionModelMat()
    {
        return glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    }

    std::vector<char> RenderPartBase::LoadShader(const std::string& fileName)
	{
        auto filePath = "../../Mortal/Shader/generated/spv/" + fileName + ".spv";
        std::ifstream file(filePath, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
	}

    vk::Buffer RenderPartBase::CreateBufferExclusive(vk::DeviceSize size, vk::BufferUsageFlags flags)
    {
        vk::BufferCreateInfo createInfo({}, size, flags, vk::SharingMode::eExclusive);
        return m_RenderingInfo.device.GetDevice().createBuffer(createInfo);
    }

    vk::DeviceMemory RenderPartBase::CreateMemoryAndBind_Buffer(const std::vector<vk::Buffer>& buffers, vk::MemoryPropertyFlags flags)
    {
        auto bufferCount = buffers.size();
        auto device = m_RenderingInfo.device.GetDevice();
        std::vector<vk::MemoryRequirements> requirements(bufferCount);
        vk::DeviceSize size = 0;
        for (int i = 0; i < bufferCount; i++) {
            requirements[i] = device.getBufferMemoryRequirements(buffers[i]);
            size += requirements[i].size;
        }
        auto memoryTypeIndex = m_RenderingInfo.device.FindMemoryIndex(requirements, flags);
        vk::MemoryAllocateInfo allocateInfo(size, memoryTypeIndex);
        auto memory = device.allocateMemory(allocateInfo);
        uint32_t offset = 0;
        for (int i = 0; i < bufferCount; i++) {
            device.bindBufferMemory(buffers[i], memory, offset);
            offset += requirements[i].size;
        }
        return memory;
    }

    vk::Image RenderPartBase::CreateImageExclusive()
    {
        return vk::Image();
    }

    vk::DeviceMemory RenderPartBase::CreateMemoryAndBind_Image(vk::Image& image, vk::MemoryPropertyFlags flags)
    {
        auto device = m_RenderingInfo.device.GetDevice();
        vk::MemoryRequirements requires;
        requires = device.getImageMemoryRequirements(image);

        uint32_t index = m_RenderingInfo.device.FindMemoryIndex(std::vector<vk::MemoryRequirements>{requires}, flags);
        vk::MemoryAllocateInfo ImageAllocateInfo(requires.size, index);
        vk::DeviceMemory ImageMemory = device.allocateMemory(ImageAllocateInfo);
        device.bindImageMemory(image, ImageMemory, 0);
        return ImageMemory;
    }

    vk::ShaderModule RenderPartBase::CreateShaderModule(const std::string& fileName)
    {   
        auto bcode = LoadShader(fileName);
        auto vertShaderModule = m_RenderingInfo.device.GetDevice().createShaderModule(vk::ShaderModuleCreateInfo({}, bcode.size(), reinterpret_cast<uint32_t*>(bcode.data())));
        return vertShaderModule;
    }

    PrepareModelInfo RenderPartBase::PrepareModel(const std::string& file)
    {
        PrepareModelInfo ret;
        auto modelInfo = LoadObjModel(file);
        auto verteices_size = modelInfo.vertices.size() * sizeof(modelInfo.vertices[0]);
        auto indices_size = modelInfo.indeices.size() * sizeof(modelInfo.indeices[0]);
        ret.modelInfo = modelInfo;
        
        //vertex
        //ret.vertexBuffer = CreateBufferExclusive(verteices_size, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst);
        //ret.vertexMemory = CreateMemoryAndBind_Buffer(std::vector<vk::Buffer>{ ret.vertexBuffer, ret.indexBuffer }, vk::MemoryPropertyFlagBits::eDeviceLocal);
        vk::BufferCreateInfo vertexCreateInfo({}, verteices_size, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::SharingMode::eExclusive);
        auto& c_vertexCreateInfo = static_cast<VkBufferCreateInfo>(vertexCreateInfo);
        VmaAllocationCreateInfo vertexAllocationCreateInfo{};
        vertexAllocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        VmaAllocation vertexAllocation;
        VkBuffer vertexBuffer;
        vmaCreateBuffer(m_VmaAllocator, &c_vertexCreateInfo, &vertexAllocationCreateInfo, &vertexBuffer, &vertexAllocation, nullptr);
        
        ret.vertexBuffer = vertexBuffer;
        ret.vertexMemory = vertexAllocation->GetMemory();

        //index
        //ret.indexBuffer = CreateBufferExclusive(indices_size, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst);
        vk::BufferCreateInfo indexCreateInfo({}, indices_size, vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst, vk::SharingMode::eExclusive);
        auto& c_indexCreateInfo = static_cast<VkBufferCreateInfo>(indexCreateInfo);
        VmaAllocationCreateInfo indexAllocationCreateInfo{};
        indexAllocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE;
        VmaAllocation indexAllocation;
        VkBuffer indexBuffer;
        vmaCreateBuffer(m_VmaAllocator, &c_indexCreateInfo, &indexAllocationCreateInfo, &indexBuffer, &indexAllocation, nullptr);
        ret.indexBuffer = indexBuffer;


        //copy
        //create stage buffer
        //auto stagebuffer = CreateBufferExclusive(verteices_size + indices_size, vk::BufferUsageFlagBits::eTransferSrc);
        //auto stageMemory = CreateMemoryAndBind_Buffer(std::vector<vk::Buffer>{ stagebuffer }, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
        vk::BufferCreateInfo stageBufferCreateInfo({}, verteices_size > indices_size? verteices_size : indices_size, vk::BufferUsageFlagBits::eTransferSrc, vk::SharingMode::eExclusive);
        auto& c_stageBufferCreateInfo = static_cast<VkBufferCreateInfo>(stageBufferCreateInfo);
        VmaAllocationCreateInfo stageAllocationCreateInfo{};
        stageAllocationCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;
        stageAllocationCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        VmaAllocation stageAllocation;
        VkBuffer c_stageBuffer;
        vmaCreateBuffer(m_VmaAllocator, &c_stageBufferCreateInfo, &stageAllocationCreateInfo, &c_stageBuffer, &stageAllocation, nullptr);
        vk::Buffer stageBuffer = c_stageBuffer;
        auto stageMemory = stageAllocation->GetMemory();

        //mapping memory and copy
        //void* data = device.mapMemory(stageMemory, 0, verteices_size);
        //memcpy(data, modelInfo.vertices.data(), verteices_size);
        //device.unmapMemory(stageMemory);

        //data = device.mapMemory(stageMemory, verteices_size, indices_size);
        //memcpy(data, modelInfo.indeices.data(), indices_size);
        //device.unmapMemory(stageMemory);
        auto& device = m_RenderingInfo.device.GetDevice();
        auto cmd = m_RenderingInfo.command.BeginSingleCommand();

        void* data;
        vmaMapMemory(m_VmaAllocator, stageAllocation, &data);
        memcpy(data, modelInfo.vertices.data(), verteices_size);
        vmaUnmapMemory(m_VmaAllocator, stageAllocation);

        cmd.copyBuffer(stageBuffer, ret.vertexBuffer, { vk::BufferCopy(0, 0, verteices_size)});
        m_RenderingInfo.command.EndSingleCommand(cmd, m_RenderingInfo.device.GetRenderingQueue().GraphicQueue.value());


        cmd = m_RenderingInfo.command.BeginSingleCommand();

        vmaMapMemory(m_VmaAllocator, stageAllocation, &data);
        memcpy(data, modelInfo.indeices.data(), indices_size);
        vmaUnmapMemory(m_VmaAllocator, stageAllocation);

        cmd.copyBuffer(stageBuffer, ret.indexBuffer, { vk::BufferCopy(0, 0, indices_size) });
        m_RenderingInfo.command.EndSingleCommand(cmd, m_RenderingInfo.device.GetRenderingQueue().GraphicQueue.value());

        device.destroyBuffer(stageBuffer);
        return ret;
    }

    void RenderPartBase::ClearUpPrepareModel(PrepareModelInfo& info)
    {
        auto& device = m_RenderingInfo.device.GetDevice();
        device.freeMemory(info.vertexMemory);
        device.destroyBuffer(info.vertexBuffer);
        device.destroyBuffer(info.indexBuffer);
    }

    void RenderPartBase::ClearUpPrepareUniform(PrepareUniformInfo& info)
    {
        auto& device = m_RenderingInfo.device.GetDevice();
        device.unmapMemory(info.uniformMemory);
        device.freeMemory(info.uniformMemory);
        device.destroyBuffer(info.uniformBuffer);
    }

    PrepareTextureInfo RenderPartBase::PrepareTexture(const std::string& file)
    {
        auto& device = m_RenderingInfo.device.GetDevice();

        auto import_texture = LoadTexture(file);
        vk::ImageCreateInfo textureImageCreateInfo( {}, vk::ImageType::e2D, vk::Format::eR8G8B8A8Srgb, vk::Extent3D( vk::Extent2D(import_texture.texWidth, import_texture.texHeight), 1 ),
                1, 1, vk::SampleCountFlagBits::e1, vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eTransferDst, vk::SharingMode::eExclusive );

        VmaAllocationCreateInfo vmaAllocationCI{};
        vmaAllocationCI.usage = VMA_MEMORY_USAGE_AUTO;
        
        VmaAllocation textureAllocation;
        VkImage tempImage;

        auto& c_textureImageCreateInfo = static_cast<VkImageCreateInfo>(textureImageCreateInfo);
        vmaCreateImage(m_VmaAllocator, &c_textureImageCreateInfo, &vmaAllocationCI, &tempImage, &textureAllocation, nullptr);

        PrepareTextureInfo retTexture;
        retTexture.textureImage = tempImage;
        retTexture.textureMemory = textureAllocation->GetMemory();

        vk::ImageViewCreateInfo textureImageViewCreateInfo({}, retTexture.textureImage , vk::ImageViewType::e2D, vk::Format::eR8G8B8A8Srgb,
            vk::ComponentMapping(vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG, vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA),
            vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
        retTexture.textureImageView = device.createImageView(textureImageViewCreateInfo);
        vk::SamplerCreateInfo textureSamplerCreateInfo({}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear,
            vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat, vk::SamplerAddressMode::eRepeat, 0.0f, VK_TRUE, 1.0f, VK_FALSE, vk::CompareOp::eAlways,
            0.0f, 1.0f, vk::BorderColor::eFloatOpaqueBlack, VK_FALSE);
        retTexture.textureSampler = device.createSampler(textureSamplerCreateInfo);

        //Copy
        vk::BufferCreateInfo textureStageBufferInfo({}, import_texture.dataSize, vk::BufferUsageFlagBits::eTransferSrc);
        auto& c_textureStageBufferInfo = static_cast<VkBufferCreateInfo>(textureStageBufferInfo);
        VkBuffer textureStageBuffer;
        VmaAllocationCreateInfo textureStageAllocationInfo{};
        textureStageAllocationInfo.usage = VMA_MEMORY_USAGE_AUTO;
        textureStageAllocationInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT;
        VmaAllocation textureStageAllocation;
        vmaCreateBuffer(m_VmaAllocator, &c_textureStageBufferInfo, &textureStageAllocationInfo, &textureStageBuffer, &textureStageAllocation, nullptr);

        void* data;
        vmaMapMemory(m_VmaAllocator, textureStageAllocation, &data);
        memcpy(data, import_texture.data, import_texture.dataSize);
        vmaUnmapMemory(m_VmaAllocator, textureStageAllocation);

        auto command = m_RenderingInfo.command;
        auto cmd = command.BeginSingleCommand();

        vk::ImageMemoryBarrier undefineToTransferDst(vk::AccessFlagBits::eNone, vk::AccessFlagBits::eTransferWrite,
            vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal, {}, {},
            retTexture.textureImage, vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, {}, {}, {}, { undefineToTransferDst });

        cmd.copyBufferToImage(textureStageBuffer, retTexture.textureImage, vk::ImageLayout::eTransferDstOptimal, { vk::BufferImageCopy(0, import_texture.texWidth, import_texture.texHeight,
            vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1), vk::Offset3D(0, 0, 0), vk::Extent3D(import_texture.texWidth, import_texture.texHeight, 1.0f)) });

        vk::ImageMemoryBarrier transferDstToShaderRead(vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead,
            vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal, {}, {},
            retTexture.textureImage, vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
        cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, {}, {}, { transferDstToShaderRead });

        command.EndSingleCommand(cmd, m_RenderingInfo.device.GetRenderingQueue().GraphicQueue.value());
        return retTexture;
    }

    TextureInfo RenderPartBase::LoadTexture(const std::string& file)
    {
        TextureInfo Info;
        Info.data = stbi_load((std::string("../../Asset/Texture/") + file).c_str(), &Info.texWidth, &Info.texHeight, &Info.texChannels, STBI_rgb_alpha);
        if (!Info.data) {
            throw std::runtime_error("failed to load texture image!");
        }
        Info.dataSize = Info.texWidth * Info.texWidth * 4;
        return Info;
    }

    LoadedModelInfo RenderPartBase::LoadObjModel(const std::string& file)
    {
        LoadedModelInfo info;

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;

        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, file.c_str())) {
            throw std::runtime_error(warn + err);
        }

        auto verteiesSize = attrib.vertices.size() / 3;
        info.vertices.reserve(verteiesSize);
        std::unordered_map<Vertex, uint32_t> ver_tex_indeies;
        for (const auto& shape : shapes) {
            for (const auto& index : shape.mesh.indices) {
                Vertex vertex{};
                vertex.Position = {
                    attrib.vertices[3 * index.vertex_index + 0],
                    attrib.vertices[3 * index.vertex_index + 1],
                    attrib.vertices[3 * index.vertex_index + 2]
                };

                vertex.Color = { 1.0f, 1.0f, 1.0f };

                vertex.Normal = {
                    attrib.normals[3 * index.normal_index +0],
                    attrib.normals[3 * index.normal_index + 1],
                    attrib.normals[3 * index.normal_index + 2]
                };

                vertex.TexCoord = {
                    attrib.texcoords[2 * index.texcoord_index + 0],
                    1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
                };

                if (ver_tex_indeies.count(vertex) == 0) {
                    ver_tex_indeies[vertex] = info.vertices.size();
                    info.vertices.emplace_back(vertex);
                }

                info.indeices.emplace_back(ver_tex_indeies[vertex]);
            }
        }
        return info;
    }
} // namespace mortal
