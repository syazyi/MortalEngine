#pragma once
#include <string>
#include "Math/Vertex.h"
#include "Rendering/rendering.h"
#include "Rendering/rendering_system.h"
#include "Rendering/RHI/vulkan/vulkan_rhi.h"
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
        void* mapped{ nullptr };
    };

    struct PrepareTextureInfo
    {
        vk::Image textureImage;
        vk::DeviceMemory textureMemory;
        vk::ImageView textureImageView;
        vk::Sampler textureSampler;
    };

    class RenderPartBase {
    public:
        RenderPartBase(VulkanContext& info);
        virtual ~RenderPartBase() {}

        virtual void Init() = 0;
        virtual void ClearUp() = 0;
        virtual void Draw() = 0;
    protected:
        void PrepareFrame();
        void SubmitQueueSync();

        glm::mat4 GetBlendCorrectionModelMat();

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

        PrepareTextureInfo PrepareTexture(const std::string& file);
    protected:
        VulkanContext& m_RenderingInfo;
    };
} // namespace mortal
