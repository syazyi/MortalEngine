#pragma once
#include <optional>
#include "Rendering/rendering.h"
namespace mortal
{

    struct RenderingQueue
    {
        std::optional<vk::Queue> GraphicQueue;
        std::optional<uint32_t> GraphicQueueFamilyIndex;

        std::optional<vk::Queue> PresentQueue;
        std::optional<uint32_t> PresentQueueFamilyIndex;
        
        std::optional<vk::Queue> ComputeQueue;
        std::optional<uint32_t> ComputeQueueFamilyIndex;

        bool HasGraphicPresentQueue() {
            return GraphicQueue.has_value() && GraphicQueueFamilyIndex.has_value() && PresentQueue.has_value() && PresentQueueFamilyIndex.has_value();
        }
    };

    class RenderingDevice {
    public:
    //init and clear
        void SetDevice(vk::Instance& instance, vk::SurfaceKHR& surface);
        void ClearUp();

        vk::PhysicalDeviceFeatures EnablePhysicalFeature();
    //physical funciton
        [[nodiscard]] vk::PhysicalDeviceProperties GetPhysicalDeviceProperties();
        [[nodiscard]] vk::FormatProperties GetPhysicalDeviceFormatProperties(vk::Format format);
        [[nodiscard]] vk::PhysicalDeviceFeatures GetPhysicalDeviceFeature();
        [[nodiscard]] uint32_t FindMemoryIndex(std::vector<vk::MemoryRequirements>& requirements, vk::MemoryPropertyFlags flags);
        [[nodiscard]] vk::Device& GetDevice();
        [[nodiscard]] vk::PhysicalDevice& GetPhysicalDevice();
        [[nodiscard]] RenderingQueue& GetRenderingQueue();

        std::pair<vk::Format, bool> FindSupportDepthFormat(std::vector<vk::Format>& formats, vk::ImageTiling imageTiling, vk::FormatFeatureFlags features);
    private:
        void ChooseSuitablePhysicalDevice(vk::Instance& instance);
    private:
        vk::PhysicalDevice m_PhysicalDevice;
        vk::Device m_LogicDevice;
        RenderingQueue m_Queues;

        vk::SurfaceKHR m_Surface;


        inline static std::vector<const char*> s_DeviceExtensions;
    };


} // namespace mortal
