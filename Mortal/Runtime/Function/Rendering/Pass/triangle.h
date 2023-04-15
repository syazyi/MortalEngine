#pragma once
#include "Rendering/rendering_pass_base.h"
namespace mortal
{
    class TrianglePass : public RenderPassBase {
    public:
        explicit TrianglePass(RenderingSystemInfo& info);
        ~TrianglePass();

        virtual void Init() override;
        virtual void ClearUp() override;
        virtual void Draw() override;
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
    };
} // namespace mortal
