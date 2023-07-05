#pragma once
#include "Rendering/rhi.h"

#include "Rendering/rendering.h"
#include "Rendering/rendering_camera.h"
#include "Rendering/RHI/vulkan/rendering_device.h"
#include "Rendering/RHI/vulkan/rendering_window.h"
#include "Rendering/RHI/vulkan/rendering_swapchain.h"
#include "Rendering/RHI/vulkan/rendering_command.h"
#include "Rendering/RHI/api.h"
namespace mortal
{
    namespace renderAPI {


        //Instance
        struct RenderInstance_Vulkan : public RenderInstance
        {
            vk::Instance instance;
            RenderPhysicalDevice_Vulkan* physical_device;
            VkDebugUtilsMessengerEXT callback_vulkan;
        };

        struct RenderPhysicalDevice_Vulkan : public RenderPhysicalDevice
        {
            vk::PhysicalDevice physical_device;
        };

        RenderInstance* CreateInstance_Vulkan(const CreateInstanceDescriptor* desc);
        void FreeInstance_Vulkan(const RenderInstance* ri);
        //End of Instance

        // Physical(adapter)
        void EnumPhysicalDevice_Vulkan(RenderInstance* ri, const RenderPhysicalDevice* rpd);
        //End of Physical(adapter)

    }//namespace renderAPI

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
        //virtual void PrepareContext() override;

        void ReCreateSwapchain();
    private:
        void CreateInstance();
        bool CheckValidtionLayer();
        std::vector<const char*> GetRequireExtensions();
        void SetDebugCallBack();

        void CreateWindowSurface();
        void CreateDevice();
        void CreateSwapchain();
        void CreateCommandPool();
        void CreateSynchronization();
        void CreateGlobalDescription();

        void DestroyInstance();
    public:
        vk::Instance m_Instance;
        VkDebugUtilsMessengerEXT callback;
        SynchronizationGlobal m_Synchronizations;
        VulkanContext m_Context;
    };// classVulkanRHI

    template<typename T>
    T GetFunction(const char* funcName, vk::Instance instance) {
        auto func = reinterpret_cast<T>(instance.getProcAddr(funcName));
        if (!func) {
            throw "error";
        }
        return func;
    }

    template<typename T, typename... Args>
    void GetAndExecuteFunction(const char* funcName, vk::Instance instance, Args&&... args) {
        auto func = reinterpret_cast<T>(instance.getProcAddr(funcName));
        func(instance, std::forward<Args>(args)...);
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);











} // namespace mortal
