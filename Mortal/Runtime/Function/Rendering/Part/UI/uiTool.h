#pragma once
#include "Rendering/rendering_system.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#ifdef _DEBUG
#define IMGUI_VULKAN_DEBUG_REPORT
#endif

namespace mortal
{
    class UITool{
    public:
        UITool(RenderingSystemInfo& info) : m_RenderingInfo(info) {}
        ~UITool() {}

        void InitUI(vk::RenderPass render_pass);
        void Draw(vk::CommandBuffer drawCmd, std::function<void(void)> uiFunc);
        void ClearUpUI();
    private:
        RenderingSystemInfo& m_RenderingInfo;
        vk::DescriptorPool m_UIPool;
    };
} // namespace mortal
