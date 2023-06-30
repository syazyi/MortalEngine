#pragma once 
#include "Mortal.h"

#include<memory>

#include "Layer/Layer.h"
#include "rendering.h"
#include "RHI/vulkan/rendering_device.h"
#include "RHI/vulkan/rendering_window.h"
#include "RHI/vulkan/rendering_swapchain.h"
#include "RHI/vulkan/rendering_command.h"
#include "rendering_camera.h"
#include "Rendering/RHI/vulkan/vulkan_rhi.h"
namespace mortal
{
    class RenderPartBase;

    class MORTAL_API RenderingSystem : public Layer{
    public:
        ~RenderingSystem();
        RenderingSystem(const RenderingSystem&) = delete;
        RenderingSystem& operator=(const RenderingSystem&) = delete;

        static RenderingSystem* GetInstance() {
            static RenderingSystem* instance = new RenderingSystem;
            return instance;
        }

        void SetUpRHI();        
        inline std::shared_ptr<RHI> GetRHIInfo();

        virtual void OnUpdate() override;
        virtual void OnEvent(Event& e) override;

        template<typename T, typename = std::enable_if_t<std::is_base_of_v<RenderPartBase, T>>>
        void AddRenderPart(T* part) {
            m_RenderParts.push_back(std::unique_ptr<T>(part));
        }
    private:
        RenderingSystem();
        void AddRenderPasses();
        void CameraMove();
    private:
        std::shared_ptr<RHI> m_RHIContext;
        std::vector<std::unique_ptr<RenderPartBase>> m_RenderParts;
    };

    std::shared_ptr<RHI> RenderingSystem::GetRHIInfo() {
        return m_RHIContext;
    }
} // namespace mortal
