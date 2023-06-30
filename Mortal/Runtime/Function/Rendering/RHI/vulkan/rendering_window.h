#pragma once 
#include "Rendering/rendering.h"

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
		[[nodiscard]] vk::Extent2D GetExtent2D();
		[[nodiscard]] MortalWindowType* GetWindow();
	private:
		vk::SurfaceKHR m_Surface;

		MortalWindowType* m_windowRef;
	};



}