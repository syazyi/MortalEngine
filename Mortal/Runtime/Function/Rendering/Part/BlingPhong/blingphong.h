#pragma once 
#include "Rendering/RHI/vulkan/rendering_part_base.h"
#include "Rendering/Part/UI/uiTool.h"
namespace mortal
{
    class BlingPhong : public RenderPartBase{
    public:
        struct MVP {
            glm::mat4 model;
            glm::mat4 view;
            glm::mat4 proj;
            glm::mat4 normalMat;
            glm::vec3 lightPos;
        };

        struct BlingPhongMaterial
        {
            glm::vec3 LightColor{1.0f, 1.0f, 1.0f};
            int q{ 256 };
            glm::vec3 Ka{ 0.1f, 0.1f, 0.1f };
            float constant{ 1.0f };
            glm::vec3 Kd{ 0.5f, 0.5f, 0.5f };
            float linear{ 0.09f };
            glm::vec3 Ks{ 1.0f, 1.0f, 1.0f };
            float quadratic{ 0.032f };
        };

        BlingPhong(VulkanContext& info);
        ~BlingPhong();

        virtual void Init() override;
        virtual void ClearUp() override;
        virtual void Draw() override;

    private:
        UITool m_UITool;
    private:
        //data
        LoadedModelInfo m_ModelInfo;
        MVP mvp;
        //Buffer and Image
        vk::Buffer m_VertexBuffer;
        vk::Buffer m_IndexBuffer;
        vk::DeviceMemory m_VertexIndexMemory;

        vk::Image m_TextureImage;
        vk::DeviceMemory m_TextureImageMemory;
        vk::ImageView m_TextureImageView;
        vk::Sampler m_TextureSampler;

        vk::Image m_DepthImage;
        vk::DeviceMemory m_DepthImageMemory;
        vk::ImageView m_DepthImageView;

        vk::Buffer m_MvpBuffer;
        vk::DeviceMemory m_MvpMemory;
        void* m_MvpData;

        //descriptor 
        vk::DescriptorSetLayout m_MvpAndSamplerSetLayout;
        std::vector<vk::DescriptorSet> m_MvpAndSamplerSets;
        vk::DescriptorPool m_DescriptorPool;

        //render pass
        vk::RenderPass m_BlingPhongPass;
        //framebuffer
        std::vector<vk::Framebuffer> m_Framebuffers;
        //blingphong pipeline
        vk::Pipeline m_BlingPhongPipeline;
        vk::PipelineLayout m_BlingPhongPipelineLayout;
        //UI info
        BlingPhongMaterial materialInfo;

        //Skybox
        LoadedModelInfo m_SkyboxModel;
        MVP skyboxMvp;
        vk::Buffer m_SkyboxMvpBuffer;
        vk::DeviceMemory m_SkyboxMvpMemory;
        void* m_SkyboxMvpData;

        vk::Buffer m_SkyboxVertexBuffer;
        vk::Buffer m_SkyboxIndexBuffer;
        vk::DeviceMemory m_SkyboxVertexIndexMemory;

        vk::Image m_SkyboxTexture;
        vk::DeviceMemory m_SkyboxMemory;
        vk::ImageView m_SkyboxView;
        vk::Sampler m_SkyboxSampler;

        vk::Pipeline m_SkyboxPipeline;
        vk::PipelineLayout m_SkyboxPipelineLayout;
        std::vector<vk::DescriptorSet> m_SkyboxDescriptorSets;
    };
    
} // namespace mortal
