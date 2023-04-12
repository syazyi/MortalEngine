#include "rendering_system.h"
#include "Rendering/rendering_device.h"
#include "Window/WindowsWindow.h"
namespace mortal
{
    RenderingSystem::RenderingSystem() : Layer("Render Layer")
    {
        SetUpVulkan();
    }

    RenderingSystem::~RenderingSystem()
    {
        ClearUpVulkan();
    }

    void RenderingSystem::SetUpVulkan()
    {
        
        m_Info.device.SetDevice(WindowsWindow::GetWindow());
    }

    void RenderingSystem::ClearUpVulkan()
    {
        m_Info.device.ClearUpDevice();
    }

} // namespace mortal
