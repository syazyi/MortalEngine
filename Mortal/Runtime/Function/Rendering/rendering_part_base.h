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

    struct PrepareModelInfo
    {
        LoadedModelInfo modelInfo;
        vk::Buffer vertexBuffer;
        vk::Buffer indexBuffer;
        vk::DeviceMemory vertexMemory;
    };

    struct PrepareUniformInfo {
        vk::Buffer uniformBuffer;
        vk::DeviceMemory uniformMemory;
        void* mapped;
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
        vk::DeviceMemory CreateMemoryAndBind_Buffer(const std::vector<vk::Buffer>& buffers, vk::MemoryPropertyFlags flags);
        vk::Image CreateImageExclusive();
        vk::DeviceMemory CreateMemoryAndBind_Image(vk::Image& image, vk::MemoryPropertyFlags flags);
        vk::ShaderModule CreateShaderModule(const std::string& fileName);

        //if this function is used  , Prepare for a decrease in flexibility
        PrepareModelInfo PrepareModel(const std::string& file);
        void ClearUpPrepareModel(PrepareModelInfo& info);

        //if this function is used  , Prepare for a decrease in flexibility
        template<typename T>
        PrepareUniformInfo PrepareUniform() {
            PrepareUniformInfo  ret;
            ret.uniformBuffer = CreateBufferExclusive(sizeof(T), vk::BufferUsageFlagBits::eUniformBuffer);
            ret.uniformMemory = CreateMemoryAndBind_Buffer(std::vector<vk::Buffer>{ ret.uniformBuffer }, vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
            auto device = m_RenderingInfo.device.GetDevice();
            ret.mapped = device.mapMemory(ret.uniformMemory, 0, sizeof(T));
            return ret;
        }
        void ClearUpPrepareUniform(PrepareUniformInfo& info);
    protected:
        RenderingSystemInfo& m_RenderingInfo;
    };
} // namespace mortal
