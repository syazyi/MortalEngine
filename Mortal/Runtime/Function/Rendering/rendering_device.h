#pragma once
#include <optional>
#include "Rendering/rendering.h"
namespace mortal
{

    struct RenderingQueue
    {
        std::optional<vk::Queue> GraphicPresentQueue;
        std::optional<uint32_t> GraphicPresenQueueFamilyIndex;
        std::optional<vk::Queue> ComputeQueue;
        std::optional<uint32_t> ComputeQueueFamilyIndex;

        bool HasGraphicPresentQueue() {
            return GraphicPresentQueue.has_value() && GraphicPresenQueueFamilyIndex.has_value();
        }
    };

    class RenderingDevice {
    public:
    //init and clear
        void SetDevice(MortalWindowType* window);
        void ClearUpDevice();

        vk::PhysicalDeviceFeatures EnablePhysicalFeature();
    //physical funciton
        [[nodiscard]] vk::PhysicalDeviceProperties GetPhysicalDevicePropertires();
        [[nodiscard]] vk::PhysicalDeviceFeatures GetPhysicalDeviceFeature();
        [[nodiscard]] vk::Device& GetDevice();
        [[nodiscard]] RenderingQueue& GetRenderingQueue();

        void SetDebugCallBack();
    private:
        std::vector<const char*> GetRequireExtensions();
        void ChooseSuitablePhysicalDevice();

        /*
            Set Validation content, when error or warning ... occupy,will call debugCallback;
        */
        bool CheckValidtionLayer();
        static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, 
            VkDebugUtilsMessageTypeFlagsEXT messageType, 
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, 
            void* pUserData);

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
            auto func =  reinterpret_cast<T>(m_Instance.getProcAddr(funcName));
            func(m_Instance, std::forward<Args>(args)...);
        }

    private:
        vk::Instance m_Instance;
        vk::PhysicalDevice m_PhysicalDevice;
        vk::Device m_LogicDevice;
        RenderingQueue m_Queues;

        vk::SurfaceKHR m_Surface;

        VkDebugUtilsMessengerEXT callback;
        inline static std::vector<const char*> s_LayerNames;
        inline static std::vector<const char*> s_DeviceExtensions;
    };


} // namespace mortal
