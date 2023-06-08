#include "shadow_mapping.h"

namespace mortal{

	ShadowPart::ShadowPart(RenderingSystemInfo& renderinfo) : RenderPartBase(renderinfo), m_UITool(renderinfo)
	{
		Init();
		//m_UITool.InitUI(m_ScenePass);
	}

	ShadowPart::~ShadowPart()
	{
		//m_UITool.ClearUpUI();
		ClearUp();
	}

	void ShadowPart::Init()
	{
		auto& device = m_RenderingInfo.device.GetDevice();
		auto depthFormatInfo = m_RenderingInfo.device.FindSupportDepthFormat(std::vector<vk::Format>{ vk::Format::eD24UnormS8Uint, vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint }, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
		auto extent2D = m_RenderingInfo.window.GetExtent2D();
		//prepare vertex info and image
		{
			m_SceneModel = PrepareModel("../../Asset/Model/TestScene.obj");
			m_MvpInfo = PrepareUniform<MVP>();

			mvp.model = GetBlendCorrectionModelMat();
			mvp.view = m_RenderingInfo.m_Camera.GetView();
			mvp.proj = glm::perspective(glm::radians(45.f), (float)extent2D.width / (float)extent2D.height, 0.1f, 100.f);
			mvp.proj[1][1] *= -1;

			//depth image
			vk::ImageCreateInfo lightDepthImageCI({}, vk::ImageType::e2D, depthFormatInfo.first, vk::Extent3D(ShadowMapWidth, ShadowMapHeight, 1), 1, 1, vk::SampleCountFlagBits::e1,
				vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled, vk::SharingMode::eExclusive);
			m_LightDepthImage = device.createImage(lightDepthImageCI);
			m_LightDepthMemroy = CreateMemoryAndBind_Image(m_LightDepthImage, vk::MemoryPropertyFlagBits::eDeviceLocal);
			vk::ImageViewCreateInfo depthImageViewCreateInfo({}, m_LightDepthImage, vk::ImageViewType::e2D, depthFormatInfo.first,
				vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1));
			m_LightDepthImageView = device.createImageView(depthImageViewCreateInfo);

			vk::SamplerCreateInfo shadowMapSamplerCI({}, vk::Filter::eLinear, vk::Filter::eLinear, vk::SamplerMipmapMode::eLinear,
				vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, vk::SamplerAddressMode::eClampToEdge, 0.0f, VK_TRUE, 1.0f, VK_FALSE, vk::CompareOp::eNever, 0.0f, 1.0f, 
				vk::BorderColor::eFloatOpaqueWhite);
			m_ShadowMapSampler = device.createSampler(shadowMapSamplerCI);

			vk::ImageCreateInfo sceneDepthImageCI({}, vk::ImageType::e2D, depthFormatInfo.first, vk::Extent3D(extent2D.width, extent2D.height, 1), 1, 1, vk::SampleCountFlagBits::e1,
				vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::SharingMode::eExclusive);
			m_SceneDepthImage = device.createImage(sceneDepthImageCI);
			m_SceneDepthMemroy = CreateMemoryAndBind_Image(m_SceneDepthImage, vk::MemoryPropertyFlagBits::eDeviceLocal);
			vk::ImageViewCreateInfo sceneImageViewCreateInfo({}, m_SceneDepthImage, vk::ImageViewType::e2D, depthFormatInfo.first,
				vk::ComponentMapping(), vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eDepth, 0, 1, 0, 1));
			m_SceneDepthImageView = device.createImageView(sceneImageViewCreateInfo);
		}

		//Prepare descriptor 
		{
			std::vector<vk::DescriptorSetLayoutBinding> bindings{
				vk::DescriptorSetLayoutBinding(0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex),
				vk::DescriptorSetLayoutBinding(1, vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment)
			};
			vk::DescriptorSetLayoutCreateInfo MvpAndSamplerlayoutCreateInfo({}, bindings);
			m_DescriptorSetLayout = device.createDescriptorSetLayout(MvpAndSamplerlayoutCreateInfo);

			std::vector<vk::DescriptorPoolSize> poolSizes{
				vk::DescriptorPoolSize(vk::DescriptorType::eUniformBuffer, 1),
				vk::DescriptorPoolSize(vk::DescriptorType::eCombinedImageSampler, 1)
			};
			vk::DescriptorPoolCreateInfo poolCreateInfo({}, 1, poolSizes);
			m_DescriptorPool = device.createDescriptorPool(poolCreateInfo);

			vk::DescriptorSetAllocateInfo setAllocateInfo(m_DescriptorPool, m_DescriptorSetLayout);
			m_DescriptorSets = device.allocateDescriptorSets(setAllocateInfo);
			
			vk::DescriptorBufferInfo MvpbufferInfo(m_MvpInfo.uniformBuffer, 0, sizeof(MVP));
			vk::DescriptorImageInfo samplerImageInfo(m_ShadowMapSampler, m_LightDepthImageView, vk::ImageLayout::eDepthStencilReadOnlyOptimal);
			device.updateDescriptorSets({
				vk::WriteDescriptorSet(m_DescriptorSets[0], 0, 0, vk::DescriptorType::eUniformBuffer, {}, MvpbufferInfo),
				vk::WriteDescriptorSet(m_DescriptorSets[0], 1, 0, vk::DescriptorType::eCombinedImageSampler, samplerImageInfo)
				}, {});
		}

		//prepare renderpass 
		{
			vk::ImageLayout transitionDepthLayout = vk::ImageLayout::eDepthReadOnlyOptimal;
			vk::ImageLayout beUsedDepthLayout = vk::ImageLayout::eDepthAttachmentOptimal;
			if (depthFormatInfo.second) {
				transitionDepthLayout = vk::ImageLayout::eDepthStencilReadOnlyOptimal;
				beUsedDepthLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
			}
			vk::AttachmentDescription attachDes_Depth({}, depthFormatInfo.first, vk::SampleCountFlagBits::e1,
				vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare, 
				vk::ImageLayout::eUndefined, transitionDepthLayout);
			vk::AttachmentReference lightDepthAttachRef(0, beUsedDepthLayout);
			vk::SubpassDescription ligthDepthSubpassDes({}, vk::PipelineBindPoint::eGraphics, 0u, {}, 0u, {}, {}, &lightDepthAttachRef);

			std::array<vk::SubpassDependency, 2> ligthDepthSubpassDeps{
				vk::SubpassDependency(VK_SUBPASS_EXTERNAL, 0, vk::PipelineStageFlagBits::eFragmentShader, vk::PipelineStageFlagBits::eEarlyFragmentTests, vk::AccessFlagBits::eShaderRead, vk::AccessFlagBits::eDepthStencilAttachmentWrite, vk::DependencyFlagBits::eByRegion),
				vk::SubpassDependency(0, VK_SUBPASS_EXTERNAL, vk::PipelineStageFlagBits::eLateFragmentTests, vk::PipelineStageFlagBits::eFragmentShader, vk::AccessFlagBits::eDepthStencilAttachmentWrite, vk::AccessFlagBits::eShaderRead, vk::DependencyFlagBits::eByRegion)
			};
			vk::RenderPassCreateInfo shadowMapRenderPassCI({}, attachDes_Depth, ligthDepthSubpassDes, ligthDepthSubpassDeps);
			m_ShadowMapCreatePass = device.createRenderPass(shadowMapRenderPassCI);

			vk::FramebufferCreateInfo ligthDepthFrameBufferCI({}, m_ShadowMapCreatePass, m_LightDepthImageView, ShadowMapWidth, ShadowMapHeight, 1);
			m_LightDepthFramebuffer = device.createFramebuffer(ligthDepthFrameBufferCI);

			vk::AttachmentDescription attachDes_Color({}, m_RenderingInfo.swapchain.GetSurfaceDetail().SurfaceFormats.format, vk::SampleCountFlagBits::e1,
				vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
				vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR);
			attachDes_Depth = vk::AttachmentDescription ({}, depthFormatInfo.first, vk::SampleCountFlagBits::e1,
				vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eDontCare, vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
				vk::ImageLayout::eUndefined, beUsedDepthLayout);
			std::array<vk::AttachmentDescription, 2> attachDes{
				attachDes_Color, attachDes_Depth
			};
			vk::AttachmentReference attachRef_Color(0, vk::ImageLayout::eColorAttachmentOptimal);
			vk::AttachmentReference attachRef_Depth(1, beUsedDepthLayout);
			vk::SubpassDescription sceneSubpass({}, vk::PipelineBindPoint::eGraphics, {}, attachRef_Color, {}, &attachRef_Depth);
			vk::SubpassDependency subPassDependency(VK_SUBPASS_EXTERNAL, 0, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eColorAttachmentOutput,
				vk::AccessFlagBits::eNone, vk::AccessFlagBits::eColorAttachmentWrite);
			//std::array<vk::SubpassDependency, 2> subPassDependencys{
			//	vk::SubpassDependency(VK_SUBPASS_EXTERNAL, 0, vk::PipelineStageFlagBits::eLateFragmentTests, vk::PipelineStageFlagBits::eFragmentShader, vk::AccessFlagBits::eDepthStencilAttachmentWrite, vk::AccessFlagBits::eShaderRead, vk::DependencyFlagBits::eByRegion),
			//	vk::SubpassDependency(0, VK_SUBPASS_EXTERNAL, vk::PipelineStageFlagBits::eColorAttachmentOutput, vk::PipelineStageFlagBits::eTopOfPipe, vk::AccessFlagBits::eColorAttachmentWrite, vk::AccessFlagBits::eMemoryWrite, vk::DependencyFlagBits::eByRegion)
			//};
			vk::RenderPassCreateInfo sceneRenderPassCI({}, attachDes, sceneSubpass, subPassDependency);
			m_ScenePass = device.createRenderPass(sceneRenderPassCI);

			auto presentImageViews = m_RenderingInfo.swapchain.GetSwapChainImageViews();
			for (int i = 0; i < presentImageViews.size(); i++) {
				std::array<vk::ImageView, 2> attachments{
					presentImageViews[i],
					m_SceneDepthImageView
				};
				vk::FramebufferCreateInfo frameCI({}, m_ScenePass, attachments, extent2D.width, extent2D.height, 1);
				m_SceneFramebuffers.push_back(device.createFramebuffer(frameCI));
			}
		}

		//prepare pipeline
		{
			vk::PushConstantRange lightMVPPC(vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4));
			vk::PipelineLayoutCreateInfo lightDepthPipelineLayoutCI({}, {}, lightMVPPC);
			m_ShadowMapPipelineLayout = device.createPipelineLayout(lightDepthPipelineLayoutCI);

			std::array<vk::PushConstantRange, 1> lightPushConstants{
				vk::PushConstantRange(vk::ShaderStageFlagBits::eVertex, 0, sizeof(glm::mat4)),
			};
			vk::PipelineLayoutCreateInfo scenePipelineLayoutCI({}, m_DescriptorSetLayout, lightPushConstants);
			m_ScenePipelineLayout = device.createPipelineLayout(scenePipelineLayoutCI);

			vk::VertexInputBindingDescription binddes(0, sizeof(Vertex));
			vk::VertexInputAttributeDescription attrDes_Pos(0, 0, vk::Format::eR32G32B32Sfloat, 0);
			vk::VertexInputAttributeDescription attrDes_Color(1, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, Color));
			vk::VertexInputAttributeDescription attrDes_Tex(2, 0, vk::Format::eR32G32Sfloat, offsetof(Vertex, TexCoord));
			vk::VertexInputAttributeDescription attrDes_Normal(3, 0, vk::Format::eR32G32B32Sfloat, offsetof(Vertex, Normal));
			std::vector<vk::VertexInputAttributeDescription> attrDes{
				attrDes_Pos, attrDes_Color, attrDes_Tex, attrDes_Normal
			};
			auto vertexInput = vk::PipelineVertexInputStateCreateInfo({}, binddes, attrDes);

			auto inputAssemblyState = vk::PipelineInputAssemblyStateCreateInfo({}, vk::PrimitiveTopology::eTriangleList, VK_FALSE);
			vk::Viewport viewprot(0, 0, extent2D.width, extent2D.height, 0.0f, 1.0f);
			vk::Rect2D scissor({ 0, 0 }, { extent2D.width, extent2D.height });
			auto viewportState = vk::PipelineViewportStateCreateInfo({}, viewprot, scissor);
			auto rasterizationState = vk::PipelineRasterizationStateCreateInfo({}, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, vk::CullModeFlagBits::eBack, vk::FrontFace::eCounterClockwise,
				VK_FALSE, {}, {}, {}, 1.0f);
			auto multisampleState = vk::PipelineMultisampleStateCreateInfo({}, vk::SampleCountFlagBits::e1, VK_FALSE);
			auto depthStencilState = vk::PipelineDepthStencilStateCreateInfo({}, VK_TRUE, VK_TRUE, vk::CompareOp::eLessOrEqual, VK_FALSE, VK_FALSE);

			vk::PipelineColorBlendAttachmentState state(VK_FALSE, vk::BlendFactor::eSrcAlpha, vk::BlendFactor::eOneMinusSrcAlpha, vk::BlendOp::eAdd,
				vk::BlendFactor::eZero, vk::BlendFactor::eOne, vk::BlendOp::eAdd,
				vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA);
			auto colorBlendState = vk::PipelineColorBlendStateCreateInfo({}, VK_FALSE, vk::LogicOp::eCopy, state);

			std::vector<vk::DynamicState> dynamicStates{ vk::DynamicState::eViewport, vk::DynamicState::eScissor };
			vk::PipelineDynamicStateCreateInfo dynamicState({}, dynamicStates);

			auto sceneVertShaderModule = CreateShaderModule("ShadowMap/Scene_vert");
			auto sceneFragShaderModule = CreateShaderModule("ShadowMap/Scene_frag");
			std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages{
				vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eVertex, sceneVertShaderModule, "main"),
				vk::PipelineShaderStageCreateInfo({}, vk::ShaderStageFlagBits::eFragment, sceneFragShaderModule, "main")
			};

			vk::GraphicsPipelineCreateInfo ScenePipelineCI({}, shaderStages, &vertexInput, &inputAssemblyState, nullptr, &viewportState, &rasterizationState,
				&multisampleState, &depthStencilState, &colorBlendState, &dynamicState, m_ScenePipelineLayout, m_ScenePass, 0, nullptr, -1);
			m_ScenePipeline = device.createGraphicsPipeline({}, ScenePipelineCI).value;

			vertexInput = vk::PipelineVertexInputStateCreateInfo({}, binddes, attrDes_Pos);
			auto lightShaderVertexModule = CreateShaderModule("ShadowMap/ShadowMap_vert");
			vk::PipelineShaderStageCreateInfo lightVertShaderStage({}, vk::ShaderStageFlagBits::eVertex, lightShaderVertexModule, "main");

			viewprot = vk::Viewport(0, 0, ShadowMapWidth, ShadowMapHeight, 0.0f, 1.0f);
			scissor = vk::Rect2D({ 0, 0 }, { ShadowMapWidth , ShadowMapHeight });

			rasterizationState = vk::PipelineRasterizationStateCreateInfo({}, VK_FALSE, VK_FALSE, vk::PolygonMode::eFill, vk::CullModeFlagBits::eNone, vk::FrontFace::eCounterClockwise,
				VK_TRUE, {}, {}, {}, 1.0f);
			//colorBlendState = vk::PipelineColorBlendStateCreateInfo({}, VK_FALSE, vk::LogicOp::eCopy);
			std::vector<vk::DynamicState> shadowDynamicStates{ vk::DynamicState::eViewport, vk::DynamicState::eScissor, vk::DynamicState::eDepthBias };
			vk::PipelineDynamicStateCreateInfo shadowDynamicState({}, shadowDynamicStates);
			vk::GraphicsPipelineCreateInfo ShadowMapPipelineCI({}, lightVertShaderStage, & vertexInput, & inputAssemblyState, nullptr, & viewportState, & rasterizationState,
				& multisampleState, & depthStencilState, & colorBlendState, & shadowDynamicState, m_ShadowMapPipelineLayout, m_ShadowMapCreatePass, 0, nullptr, -1);
			m_ShadowMapPipeline = device.createGraphicsPipeline({}, ShadowMapPipelineCI).value;

			device.destroyShaderModule(sceneFragShaderModule);
			device.destroyShaderModule(sceneVertShaderModule);
			device.destroyShaderModule(lightShaderVertexModule);
		}
	}

	void ShadowPart::ClearUp()
	{
		auto& device = m_RenderingInfo.device.GetDevice();
		device.waitIdle();

		ClearUpPrepareUniform(m_MvpInfo);

		device.destroyImageView(m_SceneDepthImageView);
		device.freeMemory(m_SceneDepthMemroy);
		device.destroyImage(m_SceneDepthImage);

		device.destroyDescriptorPool(m_DescriptorPool);
		device.destroyDescriptorSetLayout(m_DescriptorSetLayout);

		device.destroyPipeline(m_ScenePipeline);
		device.destroyPipelineLayout(m_ScenePipelineLayout);
		for (auto& framebuffer : m_SceneFramebuffers) {
			device.destroyFramebuffer(framebuffer);
		}
		device.destroyRenderPass(m_ScenePass);

		device.destroyPipeline(m_ShadowMapPipeline);
		device.destroyPipelineLayout(m_ShadowMapPipelineLayout);
		device.destroyFramebuffer(m_LightDepthFramebuffer);
		device.destroyRenderPass(m_ShadowMapCreatePass);

		device.destroyImageView(m_LightDepthImageView);
		device.freeMemory(m_LightDepthMemroy);
		device.destroyImage(m_LightDepthImage);
		device.destroySampler(m_ShadowMapSampler);

		ClearUpPrepareModel(m_SceneModel);
	}

	void ShadowPart::Draw()
	{
		PrepareFrame();

		{
			static auto start = std::chrono::high_resolution_clock::now();
			auto end = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration<float, std::chrono::seconds::period>(end - start).count();

			//mvp.lightPos = glm::mat3(glm::rotate(glm::mat4(1.0f), glm::radians(duration * 90.f), glm::vec3(0.0f, 0.0f, 1.0f))) * glm::vec3(15.0f, 15.0f, 15.0f);
			mvp.lightPos = glm::vec3(5.0f, 5.0f, 5.0f);
			auto lightView = glm::lookAt(mvp.lightPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			auto lightProj = glm::perspective(glm::radians(60.f), (float)ShadowMapWidth / (float)ShadowMapHeight, 0.1f, 100.f);
			lightProj[1][1] *= -1;
			m_LightMVP = lightProj * lightView * mvp.model;
			lightMVPInScene = lightProj * lightView;
			
			//auto debugValue = m_LightMVP * glm::vec4(mvp.lightPos, 1.0f);

			mvp.view = m_RenderingInfo.m_Camera.GetView();
			//mvp.view = glm::lookAt(mvp.lightPos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			mvp.normal = glm::transpose(glm::inverse(mvp.view * mvp.model));
			memcpy(m_MvpInfo.mapped, &mvp, sizeof(mvp));
		}
		auto& drawCmd = m_RenderingInfo.command.GetCommandBuffers()[m_RenderingInfo.CurrentFrame];
		auto& device = m_RenderingInfo.device.GetDevice();
		auto extent2D = m_RenderingInfo.window.GetExtent2D();

		drawCmd.begin(vk::CommandBufferBeginInfo{});

		std::array<vk::ClearValue, 1> depthClears{};
		depthClears[0].setDepthStencil({1.0f, 0});
		drawCmd.beginRenderPass(vk::RenderPassBeginInfo(m_ShadowMapCreatePass, m_LightDepthFramebuffer, vk::Rect2D({ 0, 0 }, { ShadowMapWidth , ShadowMapHeight }), depthClears), vk::SubpassContents::eInline);
		drawCmd.bindPipeline(vk::PipelineBindPoint::eGraphics, m_ShadowMapPipeline);
		drawCmd.setViewport(0, vk::Viewport(0.0f, 0.0f, ShadowMapWidth, ShadowMapHeight, 0.0f, 1.0f));
		drawCmd.setScissor(0, vk::Rect2D({ 0, 0 }, { ShadowMapWidth , ShadowMapHeight }));
		drawCmd.setDepthBias(m_BiasConstant, 0.0f, m_BiasSlope);
		drawCmd.bindVertexBuffers(0, m_SceneModel.vertexBuffer, {0});
		drawCmd.bindIndexBuffer(m_SceneModel.indexBuffer, 0, vk::IndexType::eUint32);
		drawCmd.pushConstants<glm::mat4>(m_ShadowMapPipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, m_LightMVP);
		drawCmd.drawIndexed(m_SceneModel.modelInfo.indeices.size(), 1, 0, 0, 0);
		drawCmd.endRenderPass();

		std::vector<vk::ClearValue> clearValues{
			vk::ClearValue(vk::ClearColorValue(std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f})),
			vk::ClearValue(vk::ClearDepthStencilValue(1.0f, 0))
		};
		drawCmd.beginRenderPass(vk::RenderPassBeginInfo(m_ScenePass, m_SceneFramebuffers[m_RenderingInfo.nextImageIndex], vk::Rect2D({0, 0}, {extent2D.width, extent2D.height}), clearValues), vk::SubpassContents::eInline);
		drawCmd.setViewport(0, vk::Viewport(0.0f, 0.0f, extent2D.width, extent2D.height, 0.0f, 1.0f));
		drawCmd.setScissor(0, vk::Rect2D({ 0, 0 }, { extent2D.width, extent2D.height }));
		drawCmd.bindPipeline(vk::PipelineBindPoint::eGraphics, m_ScenePipeline);
		drawCmd.bindVertexBuffers(0, m_SceneModel.vertexBuffer, { 0 });
		drawCmd.bindIndexBuffer(m_SceneModel.indexBuffer, 0, vk::IndexType::eUint32);
		drawCmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_ScenePipelineLayout, 0, m_DescriptorSets, {});
		drawCmd.pushConstants<glm::mat4>(m_ScenePipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, lightMVPInScene);
		drawCmd.drawIndexed(m_SceneModel.modelInfo.indeices.size(), 1, 0, 0, 0);
		drawCmd.endRenderPass();
		drawCmd.end();

		SubmitQueueSync();
	}


}// namespace mortal