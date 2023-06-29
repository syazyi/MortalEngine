#pragma once
#include "Rendering/rendering_part_base.h"
#include "ParticleSystem/particleInfo.h"
namespace mortal
{
    static constexpr uint32_t ParticelsPatchCount = 256;
    static constexpr uint32_t ParticlesCount = ParticelsPatchCount * 1024;
    class ParticlesPart : public RenderPartBase {
    public:
        struct computeUBO {							// Compute shader uniform block object
            float deltaT;							//		Frame delta time
            float destX;							//		x position of the attractor
            float destY;							//		y position of the attractor
            int32_t particleCount = ParticlesCount;
        };

        struct SceneUBO
        {
            glm::mat4 model;
            glm::mat4 view;
            glm::mat4 proj;
        };

        ParticlesPart(VulkanContext& renderInfo);
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
        PrepareUniformInfo m_CurrentParticlesUBO;
        computeUBO m_CurrentParticlesUBOData;
        //descriptor Set infp
        vk::DescriptorPool m_ParticlesDescriptorPool;
        vk::DescriptorSetLayout m_ParticlesDescriptorSetLayout;
        std::vector<vk::DescriptorSet> m_ParticlesDescriptorSets;
        //Computer Command
        vk::CommandPool m_ComputeCommandPool;
        std::vector<vk::CommandBuffer> m_ComputeCommands;

        vk::PipelineLayout m_ParticlesPipelineLayout;
        vk::Pipeline m_ParticlesPipeline;
        //Semaphore
        vk::Semaphore m_ComputeToGraphic;
        vk::Semaphore m_GraphicToCompute;
    //graphic info
        //buffer info
        vk::Image m_DepthImage;
        vk::DeviceMemory m_DepthImageMemory;
        vk::ImageView m_DepthImageView;

        vk::Image m_ParticlesColor;
        vk::DeviceMemory m_ParticlesColorMemory;
        vk::ImageView m_ParticlesColorView;
        vk::Sampler m_ParticlesColorSampler;
        PrepareUniformInfo m_SceneUBO;
        SceneUBO m_SceneUBOData;
        //pass info
        vk::RenderPass m_RenderPass;
        std::vector<vk::Framebuffer> m_Framebuffers;
        vk::PipelineLayout m_GraphicPipelineLayout;
        vk::Pipeline m_GraphicPipeline;
        //descriptor
        vk::DescriptorSetLayout m_DescriptorSetLayout;
        std::vector<vk::DescriptorSet> m_DescriptorSets;
    };
    
} // namespace mortal
