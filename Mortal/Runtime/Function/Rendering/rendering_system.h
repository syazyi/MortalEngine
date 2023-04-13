#pragma once 
#include "Mortal.h"
#include "Layer/Layer.h"
#include "rendering.h"
#include "rendering_device.h"
#include "rendering_window.h"
#include "rendering_swapchain.h"
namespace mortal
{
    struct RenderingSystemInfo
    {
        RenderingSystemInfo() = default;
        RenderingWindow window;
        RenderingDevice device;
        RenderingSwapChain swapchain;
    };

    class MORTAL_API RenderingSystem : public Layer{
    public:
        ~RenderingSystem();
        RenderingSystem(const RenderingSystem&) = delete;
        RenderingSystem& operator=(const RenderingSystem&) = delete;

        static RenderingSystem* GetInstance(){
            static RenderingSystem* instance = new RenderingSystem;
            return instance;
        }    

        void SetUpVulkan();        
        void ClearUpVulkan();

        inline RenderingSystemInfo& GetVulkanInfo(){
            return m_Info;
        }


    private:
        RenderingSystem();

        void CreateInstance();
        void DestroyInstance();
        std::vector<const char*> GetRequireExtensions();

        /*
            Set Validation content, when error or warning ... occupy,will call debugCallback;
        */
        bool CheckValidtionLayer();
        void SetDebugCallBack();
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
            auto func = reinterpret_cast<T>(m_Instance.getProcAddr(funcName));
            func(m_Instance, std::forward<Args>(args)...);
        }
    private:
        vk::Instance m_Instance;
        VkDebugUtilsMessengerEXT callback;
        RenderingSystemInfo m_Info;

    };


} // namespace mortal
