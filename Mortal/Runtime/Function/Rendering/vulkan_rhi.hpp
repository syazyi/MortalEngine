#pragma once
#include "rhi.hpp"

#include "rendering.h"
#include "rendering_device.h"
#include "rendering_window.h"
#include "rendering_swapchain.h"
#include "rendering_command.h"
#include "rendering_camera.h"
namespace mortal
{
    struct SynchronizationGlobal
    {
        std::array<vk::Semaphore, MaxFrameInFlight> m_GetImageSemaphores;
        std::array<vk::Semaphore, MaxFrameInFlight> m_PresentSemaphores;
        std::array<vk::Fence, MaxFrameInFlight> m_FrameFences;
    };

    struct VulkanContext
    {
        VulkanContext() = default;
        RenderingSwapChain swapchain;
        RenderCommand command;
        RenderingDevice device;
        RenderingWindow window;

        uint8_t CurrentFrame{0};
        uint32_t nextImageIndex;

        //global resource
        Camera m_Camera;

        SynchronizationGlobal* SemphoreInfo;

        vk::DescriptorPool DescriptorPool;
    };


    class VulkanRHI final : public RHI{
    public:
        VulkanRHI() = default;
        ~VulkanRHI();

        virtual void Init() override;
        virtual void PrepareContext() override;
        void Clear();
    private:
        void CreateInstance();
        bool CheckValidtionLayer();
        std::vector<const char*> GetRequireExtensions();
        void SetDebugCallBack();
        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);


        template<typename T>
        T GetFunction(const char* funcName) {
            auto func = reinterpret_cast<T>(m_Instance.getProcAddr(funcName));
            if (!func) {
                throw "error";
            }
            return func;
        }

        template<typename T, typename... Args>
        void GetAndExecuteFunction(const char* funcName, Args&&... args) {
            auto func = reinterpret_cast<T>(m_Instance.getProcAddr(funcName));
            func(m_Instance, std::forward<Args>(args)...);
        }

    public:
        vk::Instance m_Instance;
        VkDebugUtilsMessengerEXT callback;
        SynchronizationGlobal m_Synchronizations;
        VulkanContext m_Context;
    };

    
} // namespace mortal
