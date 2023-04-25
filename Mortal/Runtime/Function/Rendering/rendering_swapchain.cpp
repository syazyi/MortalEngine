#include "rendering_swapchain.h"
#include "rendering_device.h"
#include "rendering_window.h"
namespace mortal
{
    void RenderingSwapChain::Create(RenderingDevice& rDevice, RenderingWindow& rWindow)
    {
		m_SurfaceRef = &rWindow.GetSurface();
		m_DeviceRef = &rDevice;
		{
			auto& pDevice = rDevice.GetPhysicalDevice();

			m_SurfaceDetail.SurfaceCapabilities = pDevice.getSurfaceCapabilitiesKHR(*m_SurfaceRef);
			auto surfaceFormats = pDevice.getSurfaceFormatsKHR(*m_SurfaceRef);
			bool FindFormat = false;
			for (auto& surfaceFormat : surfaceFormats) {
				if (surfaceFormat.format == vk::Format::eB8G8R8A8Srgb && surfaceFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
					m_SurfaceDetail.SurfaceFormats = surfaceFormat;
					FindFormat = true;
					break;
				}
			}
			if (!FindFormat) {
				m_SurfaceDetail.SurfaceFormats = surfaceFormats[0];
			}

			auto presentModes = pDevice.getSurfacePresentModesKHR(*m_SurfaceRef);
			bool IsMailBox = false;
			bool IsFifo = false;
			for (auto& presentMode : presentModes) {
				if (presentMode == vk::PresentModeKHR::eFifo) {
					IsFifo = true;
				}
				else if (presentMode == vk::PresentModeKHR::eMailbox) {
					IsMailBox = true;
				}
			}
			if (IsMailBox) {
				m_SurfaceDetail.PresentModes = vk::PresentModeKHR::eMailbox;
			}
			else if (IsFifo) {
				m_SurfaceDetail.PresentModes = vk::PresentModeKHR::eFifo;
			}
			else {
				m_SurfaceDetail.PresentModes = vk::PresentModeKHR::eImmediate;
			}
		}
		CreateSwapChain();
    }

	void RenderingSwapChain::ClearUp()
	{
		auto& device = m_DeviceRef->GetDevice();
		for (auto& imageView : m_SwapChainImageViews) {
			device.destroyImageView(imageView);
		}
		device.destroySwapchainKHR(m_SwapChain);
	}

	SurfaceDetail& RenderingSwapChain::GetSurfaceDetail()
	{
		return m_SurfaceDetail;
	}

	std::vector<vk::ImageView>& RenderingSwapChain::GetSwapChainImageViews()
	{
		return m_SwapChainImageViews;
	}

	vk::SwapchainKHR& RenderingSwapChain::GetSwapChain()
	{
		return m_SwapChain;
	}

	void RenderingSwapChain::CreateSwapChain()
	{
		//Create swapchain
		{
			auto& capabilities = m_SurfaceDetail.SurfaceCapabilities;
			uint32_t minImageCountTemp = capabilities.minImageCount + 1;
			auto maxImageCountTemp = capabilities.maxImageCount;
			if (maxImageCountTemp > 0 && maxImageCountTemp < minImageCountTemp) {
				minImageCountTemp = maxImageCountTemp;
			}

			auto extent2D = capabilities.currentExtent;
			if (extent2D.width == (std::numeric_limits<uint32_t>::max)()) {
				extent2D.width = std::clamp(extent2D.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
				extent2D.height = std::clamp(extent2D.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
			}

			auto format = m_SurfaceDetail.SurfaceFormats;

			auto& queueInfo = m_DeviceRef->GetRenderingQueue();
			std::vector<uint32_t> indices{ queueInfo.GraphicQueueFamilyIndex.value(), queueInfo.PresentQueueFamilyIndex.value() };
			vk::SharingMode sharingMode;
			if (indices[0] == indices[1]) {
				sharingMode = vk::SharingMode::eExclusive;
			}
			else {
				sharingMode = vk::SharingMode::eConcurrent;
			}
			auto presentMode = m_SurfaceDetail.PresentModes;
			vk::SwapchainCreateInfoKHR createinfo({}, *m_SurfaceRef, minImageCountTemp, format.format, format.colorSpace,
				extent2D, 1, vk::ImageUsageFlagBits::eColorAttachment, sharingMode, indices, capabilities.currentTransform, vk::CompositeAlphaFlagBitsKHR::eOpaque, 
				presentMode, VK_TRUE);
			auto& device = m_DeviceRef->GetDevice();
			m_SwapChain = device.createSwapchainKHR(createinfo);

			//get image
			m_SwapChainImages = device.getSwapchainImagesKHR(m_SwapChain);

			//create imageview
			auto size = m_SwapChainImages.size();
			m_SwapChainImageViews.resize(size);
			for (size_t i = 0; i < size; i++) {

				vk::ImageViewCreateInfo ViewCreateInfo({}, m_SwapChainImages[i], vk::ImageViewType::e2D, format.format, {}, 
					vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
				m_SwapChainImageViews[i] = device.createImageView(ViewCreateInfo);
			}
		}

	}
} // namespace mortal
