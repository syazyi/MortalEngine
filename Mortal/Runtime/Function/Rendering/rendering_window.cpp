#include "rendering_window.h"
#include "rendering_device.h"
#include "Window/WindowsWindow.h"
namespace mortal {

	void RenderingWindow::SetWindow(vk::Instance& instance)
	{

		//Get surface detail information
		if constexpr (Mortal_PlatformID == 1) {
			m_windowRef = WindowsWindow::GetWindow();
			vk::Win32SurfaceCreateInfoKHR createInfo({}, GetModuleHandle(nullptr), glfwGetWin32Window(m_windowRef));
			m_Surface = instance.createWin32SurfaceKHR(createInfo);
		}
	}

	void RenderingWindow::ClearUp(vk::Instance& instance)
	{
		instance.destroySurfaceKHR(m_Surface);
	}

	vk::SurfaceKHR& RenderingWindow::GetSurface()
	{
		return m_Surface;
	}

	//call in window when size be change
	vk::Extent2D RenderingWindow::GetExtent2D()
	{
		vk::Extent2D extent2D;
		int width;
		int height;
		glfwGetWindowSize(m_windowRef, &width, &height);

		extent2D.setWidth(static_cast<uint32_t>(width));
		extent2D.setHeight(static_cast<uint32_t>(height));

		return extent2D;
	}

}