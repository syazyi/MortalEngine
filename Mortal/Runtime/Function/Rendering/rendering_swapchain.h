#pragma once 
#include "rendering.h"
namespace mortal
{
    class RenderingDevice;
    class RenderingWindow;

    struct SurfaceDetail {
        vk::SurfaceCapabilitiesKHR SurfaceCapabilities;
        vk::SurfaceFormatKHR SurfaceFormats;
        vk::PresentModeKHR PresentModes;
    };

    class RenderingSwapChain {
    public:
        void Create(RenderingDevice& rDevice, RenderingWindow& rWindow);
        void ClearUp();
        void ReCreateSwapChain();
        SurfaceDetail& GetSurfaceDetail();
        std::vector<vk::ImageView>& GetSwapChainImageViews();
        [[nodiscard]] vk::SwapchainKHR& GetSwapChain();
    private:
        void CreateSwapChain();

        SurfaceDetail m_SurfaceDetail;

        vk::SwapchainKHR m_SwapChain;
        std::vector<vk::Image> m_SwapChainImages;
        std::vector<vk::ImageView> m_SwapChainImageViews;
        //std::vector<vk::Framebuffer> m_SwapChainFrameBuffers;

        //use recreate swapchain
        vk::SurfaceKHR* m_SurfaceRef;
        RenderingDevice* m_DeviceRef;
    };
} // namespace mortal
