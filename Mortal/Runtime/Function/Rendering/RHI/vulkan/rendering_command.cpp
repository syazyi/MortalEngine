#include "rendering_command.h"
#include "rendering_device.h"
namespace mortal
{
	void mortal::RenderCommand::SetCommandPool(RenderingDevice& rDevice)
	{
		auto& device = rDevice.GetDevice();
		m_DeviceRef = &device;
		auto& graphic_presentQueueIndex = rDevice.GetRenderingQueue().GraphicQueueFamilyIndex;
		vk::CommandPoolCreateInfo cmdCI(vk::CommandPoolCreateFlagBits::eResetCommandBuffer, graphic_presentQueueIndex.value());
		m_CommandPool = device.createCommandPool(cmdCI);
		
		m_DrawCmdbuffers = m_DeviceRef->allocateCommandBuffers(vk::CommandBufferAllocateInfo(m_CommandPool, vk::CommandBufferLevel::ePrimary, MaxFrameInFlight));
	}

	void RenderCommand::ClearUp()
	{
		m_DeviceRef->destroyCommandPool(m_CommandPool);
	}

	vk::CommandBuffer RenderCommand::BeginSingleCommand()
	{
		vk::CommandBufferAllocateInfo allInfo(m_CommandPool, vk::CommandBufferLevel::ePrimary, 1);
		vk::CommandBuffer drawCmdBuffer = m_DeviceRef->allocateCommandBuffers(allInfo)[0];

		vk::CommandBufferBeginInfo beginInfo(vk::CommandBufferUsageFlagBits::eOneTimeSubmit, {});
		drawCmdBuffer.begin(beginInfo);
		return drawCmdBuffer;
	}

	void RenderCommand::EndSingleCommand(vk::CommandBuffer& drawCmdBuffer, vk::Queue& queue)
	{
		drawCmdBuffer.end();
		vk::SubmitInfo subInfo({}, {}, drawCmdBuffer, {});
		queue.submit(subInfo);
		queue.waitIdle();
		
		m_DeviceRef->freeCommandBuffers( m_CommandPool, drawCmdBuffer);
	}
	std::vector<vk::CommandBuffer>& RenderCommand::GetCommandBuffers()
	{
		return m_DrawCmdbuffers;
	}
} // namespace mortal
