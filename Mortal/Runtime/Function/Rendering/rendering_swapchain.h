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
        void ReCreateSwapChain(MortalWindowType* window);
        SurfaceDetail& GetSurfaceDetail(RenderingDevice& rDevice);
        std::vector<vk::ImageView>& GetSwapChainImageViews();
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
