#pragma once
#include "Rendering/rendering_part_base.h"
#include "ParticleSystem/particleInfo.h"
namespace mortal
{
    static constexpr uint32_t ParticlesCount = 256 * 1024;
    class ParticlesPart : public RenderPartBase {
    public:
        ParticlesPart(RenderingSystemInfo& renderInfo);
        ~ParticlesPart();

        virtual void Init() override;
        virtual void ClearUp() override;
        virtual void Draw() override;
        std::vector<ParticlesInfo> PrepareParticles();
    private:
    //computer info
        //buffer info
        vk::Buffer m_CurrentParticlesBuffer;
        vk::DeviceMemory  m_CurrentParticlesBufferMemory;
        //descriptor Set infp
        vk::DescriptorPool m_ParticlesDescriptorPool;
        vk::DescriptorSetLayout m_ParticlesDescriptorSetLayout;
        std::vector<vk::DescriptorSet> m_ParticlesDescriptorSets;

        vk::PipelineLayout m_ParticlesPipelineLayout;
        vk::Pipeline m_ParticlesPipeline;
    //graphic info
        //buffer info

        vk::Image m_ParticlesColor;
        //pass info
        vk::RenderPass m_RenderPass;
        std::vector<vk::Framebuffer> m_Framebuffers;
        vk::PipelineLayout m_GraphicPipelineLayout;
        vk::Pipeline m_GraphicPipeline;

    };
    
} // namespace mortal
