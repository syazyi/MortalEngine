#pragma once 
#include "Mortal.h"

#include<memory>

#include "Layer/Layer.h"
#include "rendering.h"
#include "rendering_device.h"
#include "rendering_window.h"
#include "rendering_swapchain.h"
#include "rendering_command.h"
#include "rendering_camera.h"
namespace mortal
{
    class RenderPartBase;

    struct SynchronizationGlobal
    {
        std::array<vk::Semaphore, MaxFrameInFlight> m_GetImageSemaphores;
        std::array<vk::Semaphore, MaxFrameInFlight> m_PresentSemaphores;
        std::array<vk::Fence, MaxFrameInFlight> m_FrameFences;
    };

    struct RenderingSystemInfo
    {
        RenderingSystemInfo() = default;
        RenderingSwapChain swapchain;
        RenderCommand command;
        RenderingDevice device;
        RenderingWindow window;

        uint8_t CurrentFrame{0};
        uint32_t nextImageIndex;
        Camera m_Camera;
        SynchronizationGlobal* SemphoreInfo;
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

        virtual void OnUpdate() override;
        virtual void OnEvent(Event& e) override;

        template<typename T, typename = std::enable_if_t<std::is_base_of_v<RenderPartBase, T>>>
        void AddRenderPart(T* part) {
            m_RenderParts.push_back(std::unique_ptr<T>(part));
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

        void AddRenderPasses();
        void CameraMove();
    private:
        vk::Instance m_Instance;
        VkDebugUtilsMessengerEXT callback;
        RenderingSystemInfo m_Info;
        SynchronizationGlobal m_Synchronizations;
        std::vector<std::unique_ptr<RenderPartBase>> m_RenderParts;
    };


} // namespace mortal
