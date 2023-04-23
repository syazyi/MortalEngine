#pragma once
#include <string>
#include "rendering.h"
#include "rendering_system.h"
#include "Math/Vertex.h"
namespace mortal
{
    struct RenderingSystemInfo;

    struct TextureInfo
    {
        int texWidth;
        int texHeight;
        int texChannels;
        vk::DeviceSize dataSize;
        unsigned char* data{ nullptr };
    };

    struct LoadedModelInfo
    {
        std::vector<Vertex> vertices;
        std::vector<uint32_t> indeices;
    };

    class RenderPartBase {
    public:
        RenderPartBase(RenderingSystemInfo& info);
        virtual ~RenderPartBase() {}

        virtual void Init() = 0;
        virtual void ClearUp() = 0;
        virtual void Draw() = 0;
    protected:
        std::vector<char> LoadShader(const std::string& fileName);
        TextureInfo LoadTexture(const std::string& file);
        LoadedModelInfo LoadObjModel(const std::string& file);
        vk::Buffer CreateBufferExclusive(vk::DeviceSize size, vk::BufferUsageFlags flags);
        vk::DeviceMemory CreateMemoryAndBind_Buffer(std::vector<vk::Buffer>& buffers, vk::MemoryPropertyFlags flags);
        vk::Image CreateImageExclusive();
        vk::DeviceMemory CreateMemoryAndBind_Image(vk::Image& image, vk::MemoryPropertyFlags flags);
        vk::ShaderModule CreateShaderModule(const std::string& fileName);
    protected:
        RenderingSystemInfo& m_RenderingInfo;
    };
} // namespace mortal
