#pragma once 
#include "Rendering/rendering_part_base.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#ifdef _DEBUG
#define IMGUI_VULKAN_DEBUG_REPORT
#endif

namespace mortal
{
    
    class UI : public RenderPartBase{
    public:
        UI(RenderingSystemInfo& info);
        ~UI();

        virtual void Init() override;
        virtual void ClearUp() override;
        virtual void Draw() override;
    private:
        vk::DescriptorPool m_DescriptorPool;
        vk::PipelineCache m_PipelineCache;

        vk::RenderPass m_UIPass;
        std::vector<vk::Framebuffer> m_FrameBuffers;

        bool show_demo_window;

    };

} // namespace mortal
