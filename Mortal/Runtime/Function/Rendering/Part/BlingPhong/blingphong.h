#pragma once 
#include "Rendering/rendering_part_base.h"
#include "Rendering/Part/UI/uiTool.h"
namespace mortal
{
    class BlingPhong : public RenderPartBase{
    public:
        struct MVP {
            glm::mat4 model;
            glm::mat4 view;
            glm::mat4 proj;
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

        BlingPhong(RenderingSystemInfo& info);
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
    };
    
} // namespace mortal