#pragma once 

#include "Window/Window.h"
namespace mortal
{
    class MORTAL_API WindowsWindow : public Window{
    public:
        explicit WindowsWindow(const WindowCreateInfo& createInfo);
        explicit WindowsWindow(const char* title, uint32_t width, uint32_t height);
        ~WindowsWindow();

        uint32_t GetWidth() const override;
        uint32_t GetHeight() const override;

        void SetCallback(EventCallback func) override { m_createInfo.callfunc = func;  }
        void Update() override;

        bool CheckShouldBeClose() override{
            return !glfwWindowShouldClose(m_window);
        }

        [[nodiscard]] static MortalWindowType* GetWindow() {
            return m_window;
        }

    private:
        void Init();
        void ShutDown();
    private:
        WindowCreateInfo m_createInfo;

        inline static MortalWindowType* m_window{nullptr};
    };


    
} // namespace mortal
