#include "rendering_pass_base.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace mortal
{
	RenderPassBase::RenderPassBase(RenderingSystemInfo& info) : m_RenderingInfo(info)	
	{

	}

    std::vector<char> RenderPassBase::LoadShader(const std::string& fileName)
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

    vk::Buffer RenderPassBase::CreateBufferExclusive(vk::DeviceSize size, vk::BufferUsageFlags flags)
    {
        vk::BufferCreateInfo createInfo({}, size, flags, vk::SharingMode::eExclusive);
        return m_RenderingInfo.device.GetDevice().createBuffer(createInfo);
    }

    vk::DeviceMemory RenderPassBase::CreateMemoryAndBind(std::vector<vk::Buffer>& buffers, vk::MemoryPropertyFlags flags)
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

    RenderPassBase::TextureInfo RenderPassBase::LoadTexture(const std::string& file)
    {

        TextureInfo Info;
        Info.data = stbi_load((std::string("../../Asset/Texture/") + file).c_str(), &Info.texWidth, &Info.texHeight, &Info.texChannels, STBI_rgb_alpha);
        if (!Info.data) {
            throw std::runtime_error("failed to load texture image!");
        }
        Info.dataSize = Info.texWidth * Info.texWidth * 4;
        return Info;
    }
} // namespace mortal
