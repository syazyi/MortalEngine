#pragma once 
#include "Rendering/rendering_part_base.h"
#include "Rendering/Part/UI/uiTool.h"

constexpr uint32_t ShadowMapWidth = 2048;
constexpr uint32_t ShadowMapHeight = 2048;

namespace mortal
{
    class ShadowPart : public RenderPartBase {
    public:
        struct MVP {
            glm::mat4 model;
            glm::mat4 view;
            glm::mat4 proj;
            glm::mat4 normal;
            glm::vec3 lightPos;
        };

        ShadowPart(VulkanContext& renderinfo);
        ~ShadowPart();
        void Init() override;
        void ClearUp() override;
        void Draw() override;
    private:
        UITool m_UITool;

        //offscene render info
        float m_BiasConstant{ 1.25f };
        float m_BiasSlope{ 1.75f };
        PrepareModelInfo m_SceneModel;

        glm::mat4 m_LightMVP;

        vk::Image m_LightDepthImage;
        vk::ImageView m_LightDepthImageView;
        vk::DeviceMemory m_LightDepthMemroy;
        vk::Framebuffer m_LightDepthFramebuffer;
        vk::Sampler m_ShadowMapSampler;

        vk::RenderPass m_ShadowMapCreatePass;
        vk::Pipeline m_ShadowMapPipeline;
        vk::PipelineLayout m_ShadowMapPipelineLayout;

        //std::vector<vk::Framebuffer> m_LightDepthFramebuffers_Test;

        //scene
        PrepareUniformInfo m_MvpInfo;
        MVP mvp;
        glm::mat4 lightMVPInScene;

        vk::Image m_SceneDepthImage;
        vk::ImageView m_SceneDepthImageView;
        vk::DeviceMemory m_SceneDepthMemroy;
        std::vector<vk::Framebuffer> m_SceneFramebuffers;

        vk::RenderPass m_ScenePass;
        vk::Pipeline m_ScenePipeline;
        vk::PipelineLayout m_ScenePipelineLayout;

        vk::DescriptorPool m_DescriptorPool;
        vk::DescriptorSetLayout m_DescriptorSetLayout;
        std::vector<vk::DescriptorSet> m_DescriptorSets;
    };
} // namespace mortal
