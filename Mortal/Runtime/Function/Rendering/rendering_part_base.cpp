#include "rendering_part_base.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

namespace mortal
{
	RenderPartBase::RenderPartBase(RenderingSystemInfo& info) : m_RenderingInfo(info)	
	{

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

    vk::DeviceMemory RenderPartBase::CreateMemoryAndBind_Buffer(std::vector<vk::Buffer>& buffers, vk::MemoryPropertyFlags flags)
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
                    info.vertices.push_back(vertex);
                }

                info.indeices.push_back(ver_tex_indeies[vertex]);
            }
        }
        return info;
    }
} // namespace mortal
