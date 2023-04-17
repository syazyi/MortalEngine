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
        std::vector<char> LoadShader(const std::string& fileName);
        vk::Buffer CreateBufferExclusive(vk::DeviceSize size, vk::BufferUsageFlags flags);
        vk::DeviceMemory CreateMemoryAndBind(std::vector<vk::Buffer>& buffers, vk::MemoryPropertyFlags flags);
    protected:
        RenderingSystemInfo& m_RenderingInfo;
    };
} // namespace mortal
