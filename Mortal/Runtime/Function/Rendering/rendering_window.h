#include "rendering.h"

namespace mortal {
	class RenderingDevice;
	class Window;
	/*
		To create swapChain and save info.
	*/
	class RenderingWindow {
	public:
		void SetWindow(vk::Instance& instance);
		void ClearUp(vk::Instance& instance);
		[[nodiscard]] vk::SurfaceKHR& GetSurface();
		[[nodiscard]] vk::Extent2D GetExtent2D(Window* window);
	private:
		vk::SurfaceKHR m_Surface;

	};



}