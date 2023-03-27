#include "Window/WindowsWindow.h"
#include "Events/WindowEvent.h"
namespace mortal{
    


    WindowsWindow::WindowsWindow(const WindowCreateInfo& createInfo) : m_createInfo(createInfo)
    {
        Init();
    }

    WindowsWindow::WindowsWindow(const char* title, uint32_t width, uint32_t height)
    {
        m_createInfo.pTitle = title;
        m_createInfo.width = width;
        m_createInfo.height = height;
        Init();
    }

    WindowsWindow::~WindowsWindow()
    {
        ShutDown();
    }

    uint32_t WindowsWindow::GetWidth() const
    {
        return m_createInfo.width;
    }

    uint32_t WindowsWindow::GetHeight() const
    {
        return m_createInfo.height;
    }

    void WindowsWindow::Update()
    {
        glfwPollEvents();
        
    }

    void WindowsWindow::Init()
    {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);
        m_window = glfwCreateWindow(m_createInfo.width, m_createInfo.height, m_createInfo.pTitle, nullptr, nullptr);
        glfwSetWindowUserPointer(m_window, &m_createInfo);

        glfwSetWindowSizeCallback(m_window, [](GLFWwindow* window, int width, int height) {
                auto data = reinterpret_cast<WindowCreateInfo*>(glfwGetWindowUserPointer(window));
                data->width = width;
                data->height = height;

                WindowResizeEvent event(width, height);
                data->callfunc(event);
            });

        glfwSetWindowCloseCallback(m_window, [](GLFWwindow* window) {
                auto data = reinterpret_cast<WindowCreateInfo*>(glfwGetWindowUserPointer(window));
                WindosCloseEvent e;
                data->callfunc(e);
            });

        glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            auto data = reinterpret_cast<WindowCreateInfo*>(glfwGetWindowUserPointer(window));
            switch (action)
            {
                case GLFW_PRESS:
                {
                    KeyPressEvent e(key);
                    data->callfunc(e);
                    break;
                }
                case GLFW_RELEASE:
                {
                    KeyReleaseEvent e(key);
                    data->callfunc(e);
                    break;
                }
                //case GLFW_REPEAT:
                //{
                //    KeyPressEvent e;
                //    data->callfunc(e);
                //    break;
                //}
            }
            });

        //glfwSetMouseButtonCallback();
        //glfwSetScrollCallback();
        glfwSetCursorPosCallback(m_window, [](GLFWwindow* window, double xpos, double ypos) {
            auto data = reinterpret_cast<WindowCreateInfo*>(glfwGetWindowUserPointer(window));
            MouseMovedEvent e(xpos, ypos);
            data->callfunc(e);
        });

    }

    void WindowsWindow::ShutDown()
    {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }

}