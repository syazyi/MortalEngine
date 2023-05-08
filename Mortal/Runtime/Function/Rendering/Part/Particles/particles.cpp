#include "Particles.h"
#include <random>
namespace mortal
{
    ParticlesPart::ParticlesPart(RenderingSystemInfo& renderInfo) : RenderPartBase(renderInfo)
    {
        Init();
    }

    ParticlesPart::~ParticlesPart()
    {
        ClearUp();
    }

    void ParticlesPart::Init()
    {
        auto& device = m_RenderingInfo.device.GetDevice();
        auto extent2D = m_RenderingInfo.window.GetExtent2D();
        //compute prepare
        {
            auto& computeIndex = m_RenderingInfo.device.GetRenderingQueue().ComputeQueueFamilyIndex.value();
            auto& computeQueue = m_RenderingInfo.device.GetRenderingQueue().ComputeQueue.value();

            auto particles = PrepareParticles();
            auto particles_size = particles.size() * sizeof(ParticlesInfo);
            vk::BufferCreateInfo particlesBufferCI({}, particles_size, vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst,
                vk::SharingMode::eExclusive);
            m_CurrentParticlesBuffer = device.createBuffer(particlesBufferCI);
            m_CurrentParticlesBufferMemory = CreateMemoryAndBind_Buffer(std::vector<vk::Buffer>{ m_CurrentParticlesBuffer }, vk::MemoryPropertyFlagBits::eDeviceLocal);

            //ToDo: stage buffer and copy , use barrier if graphic queue isnot same with compute queue. you need an commandpool allocate compute command buffer.

            //Descriptor
            std::vector<vk::DescriptorPoolSize> poolSizes{ 
                vk::DescriptorPoolSize(vk::DescriptorType::eStorageBuffer, 1), 
                vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 1), 
            };
            vk::DescriptorPoolCreateInfo decriptorPoolCI({}, 1, poolSizes);
            m_ParticlesDescriptorPool = device.createDescriptorPool(decriptorPoolCI);
            std::vector<vk::DescriptorSetLayoutBinding> DescriptorSetLayoutBinding{
                vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute), 
                vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eCompute),
            };
            vk::DescriptorSetLayoutCreateInfo ParticlesDSLCI({}, DescriptorSetLayoutBinding);
            m_ParticlesDescriptorSetLayout = device.createDescriptorSetLayout(ParticlesDSLCI);
            vk::DescriptorSetAllocateInfo ParticlesDSA(m_ParticlesDescriptorPool, m_ParticlesDescriptorSetLayout);
            m_ParticlesDescriptorSets = device.allocateDescriptorSets(ParticlesDSA);
            //pipelinelayout and pipeline Set
            vk::PipelineLayoutCreateInfo ParticlesPLCI({}, m_ParticlesDescriptorSetLayout);
            m_ParticlesPipelineLayout = device.createPipelineLayout(ParticlesPLCI);
            vk::PipelineShaderStageCreateInfo();
            vk::ComputePipelineCreateInfo computerPCI({}, , m_ParticlesPipelineLayout);
            m_ParticlesPipeline = device.createComputePipeline(nullptr, computerPCI).value;
        }
    }

    void ParticlesPart::ClearUp()
    {
        auto& device = m_RenderingInfo.device.GetDevice();
        device.waitIdle();
        //clear up
        


        device.freeMemory(m_CurrentParticlesBufferMemory);
        device.destroyBuffer(m_CurrentParticlesBuffer);
    }

    void ParticlesPart::Draw()
    {
        //compute command
        //TODO: you need to set barrier.
        {

        }
    }

    std::vector<ParticlesInfo> ParticlesPart::PrepareParticles()
    {
        std::default_random_engine rndEngine((unsigned)time(nullptr));
        std::uniform_real_distribution<float> rndDist(-1.0f, 1.0f);

        // Initial particle positions
        std::vector<ParticlesInfo> particleBuffer(ParticlesCount);
        for (auto& particle : particleBuffer) {
            particle.Position = glm::vec3(rndDist(rndEngine), rndDist(rndEngine), rndDist(rndEngine));
            particle.Velocity = glm::vec3(0.0f);
            //particle.gradientPos.x = particle.pos.x / 2.0f;
        }
        return particleBuffer;
    }

} // namespace mortal
