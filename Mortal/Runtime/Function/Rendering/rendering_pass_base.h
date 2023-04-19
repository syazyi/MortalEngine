#pragma once
#include <string>
#include "rendering.h"
#include "rendering_system.h"
namespace mortal
{
    struct RenderingSystemInfo;
    class RenderPassBase {
    public:
        RenderPassBase(RenderingSystemInfo& info);
        virtual ~RenderPassBase() {}

        virtual void Init() = 0;
        virtual void ClearUp() = 0;
        virtual void Draw() = 0;
    protected:
        struct TextureInfo
        {
            int texWidth;
            int texHeight;
            int texChannels;
            vk::DeviceSize dataSize;
            unsigned char* data{ nullptr };
        };

        std::vector<char> LoadShader(const std::string& fileName);
        vk::Buffer CreateBufferExclusive(vk::DeviceSize size, vk::BufferUsageFlags flags);
        vk::DeviceMemory CreateMemoryAndBind(std::vector<vk::Buffer>& buffers, vk::MemoryPropertyFlags flags);
        TextureInfo LoadTexture(const std::string& file);
    protected:
        RenderingSystemInfo& m_RenderingInfo;
    };
} // namespace mortal
