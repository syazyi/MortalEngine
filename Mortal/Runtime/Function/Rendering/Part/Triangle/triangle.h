#pragma once
#include "Rendering/RHI/vulkan/rendering_part_base.h"
namespace mortal
{
    class TrianglePart : public RenderPartBase {
    public:
        using IndexType = uint16_t;

        struct UBO
        {
            glm::mat4 Model;
            glm::mat4 View;
            glm::mat4 Project;
        };
    public:
        explicit TrianglePart(VulkanContext& info);
        ~TrianglePart();

        virtual void Init() override;
        void UseRHIInit();
        virtual void ClearUp() override;
        virtual void Draw() override;
    private:

    private:
        vk::PipelineLayout m_PipelineLayout;
        vk::ShaderModule m_VertexShaderModule;
        vk::ShaderModule m_FragmentShaderModule;
        vk::Pipeline m_Pipeline;

        vk::RenderPass m_RenderPass;
        std::vector<vk::Framebuffer> m_FrameBuffers;

        //vertex buffer
        vk::Buffer m_VertexBuffer;
        vk::Buffer m_IndexBuffer;

        vk::DeviceMemory m_VertexIndexMemroy;

        vk::Buffer m_VertexBufferSecond;
        vk::DeviceMemory m_VertexSecondMemory;
        //MVP Uniform buffer
        vk::Buffer m_MVPUniformBuffer;
        vk::DeviceMemory m_MVPMemory;
        void* m_MVPData;

        //texture image
        vk::Image m_TextureImage;
        vk::DeviceMemory m_TextureMemory;

        vk::ImageView m_TextureImageView;

        vk::Sampler m_TextureSampler;

        //Depth Image
        vk::Image m_DepthImage;
        vk::DeviceMemory m_DepthImageMemory;
        vk::ImageView m_DepthImageView;
        bool SupportStencil{ true };

        //data
        std::vector<Vertex> Test_Vertices;
        std::vector<uint16_t> Test_Indices;
        UBO mvp;

        //Des
        vk::DescriptorPool m_DescriptorPool;
        vk::DescriptorSetLayout m_TriangleDescriptorSetLayout;
        std::vector<vk::DescriptorSet> m_TriangleDesCriptorSets;
    };
} // namespace mortal
