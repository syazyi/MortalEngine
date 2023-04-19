#pragma once
#include "Rendering/rendering_pass_base.h"
#include "glm/vec3.hpp"
#include "glm/gtc/matrix_transform.hpp"
namespace mortal
{
    class TrianglePass : public RenderPassBase {
    public:
        struct Vertex
        {
            glm::vec3 Position;
            glm::vec3 Color;
            glm::vec2 TexCoord;
        };

        struct UBO
        {
            glm::mat4 Model;
            glm::mat4 View;
            glm::mat4 Project;
        };
    public:
        explicit TrianglePass(RenderingSystemInfo& info);
        ~TrianglePass();

        virtual void Init() override;
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

        std::array<vk::Semaphore, MaxFrameInFlight> m_GetImageSemaphores;
        std::array<vk::Semaphore, MaxFrameInFlight> m_PresentSemaphores;
        std::array<vk::Fence, MaxFrameInFlight> m_FrameFences;

        std::vector<vk::CommandBuffer> m_DrawCmds;

        //vertex buffer
        vk::Buffer m_VertexBuffer;
        vk::Buffer m_IndexBuffer;

        vk::DeviceMemory m_VertexIndexMemroy;

        //MVP Uniform buffer
        vk::Buffer m_MVPUniformBuffer;
        vk::DeviceMemory m_MVPMemory;
        void* m_MVPData;

        //texture image
        vk::Image m_TextureImage;
        vk::DeviceMemory m_TextureMemory;

        //data
        std::vector<Vertex> Test_Vertices;
        std::vector<uint32_t> Test_Indices;
        UBO mvp;

        //Des
        vk::DescriptorPool m_DescriptorPool;
        vk::DescriptorSetLayout m_TriangleDescriptorSetLayout;
        std::vector<vk::DescriptorSet> m_TriangleDesCriptorSets;
    };
} // namespace mortal
