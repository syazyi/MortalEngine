#include "Rendering/RHI/vulkan/rendering_part_base.h"

namespace mortal
{
    class DeferredPart : public RenderPartBase{
    public:
        struct ModelUBO
        {
            glm::mat4 model;
            glm::mat4 view;
            glm::mat4 proj;
        };

        DeferredPart(VulkanContext& info);
        ~DeferredPart();
        virtual void Init() override;
        virtual void Draw() override;
        virtual void ClearUp() override;

    private:
        PrepareModelInfo m_ModelPlane;
        PrepareModelInfo m_ModelMonkeyHead;

        PrepareUniformInfo m_ModelUBO;
        ModelUBO m_ModelUBOData;

        //Descriptor
        vk::DescriptorPool m_DescriptorPool;
        vk::DescriptorSetLayout m_DescriptorSetLayout;
        std::vector<vk::DescriptorSet> m_PresentDescriptorSets;
        //RenderPass 
        vk::RenderPass m_PresentRenderPass;
        std::vector<vk::Framebuffer> m_PresentFrameBuffers;

        //Pipeline
        vk::PipelineLayout m_PresentPipelineLayout;
        vk::Pipeline m_PresentPipeline;
    };


} // namespace mortal
