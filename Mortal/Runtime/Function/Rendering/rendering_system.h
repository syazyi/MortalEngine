#pragma once 
#include "Mortal.h"
#include "Layer/Layer.h"
#include "rendering.h"
#include "Rendering/rendering_device.h"

namespace mortal
{
    struct RenderingSystemInfo
    {
        RenderingDevice device;
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

        RenderingSystemInfo m_Info;
    private:
        RenderingSystem();
    };


} // namespace mortal
