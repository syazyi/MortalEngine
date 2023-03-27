#include "WindowsWindow.h"

namespace mortal {
	Window* Window::Create(const WindowCreateInfo& info)
	{
#ifdef MORTAL_PLATFORM_WINDOWS
		return new WindowsWindow(info);
#else
		return nullptr;
#endif // MORTAL_PLATFORM_WINDOWS
	}

}