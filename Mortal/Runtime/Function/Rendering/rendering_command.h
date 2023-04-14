#include "rendering.h"
namespace mortal
{
    class RenderingDevice;

    class RenderCommand {
    public:
        void SetCommandPool(RenderingDevice& rDevice);
        void ClearUp();
        vk::CommandBuffer BeginSingleCommand();
        void EndSingleCommand(vk::CommandBuffer& drawCmdBuffer, vk::Queue& queue);
        [[nodiscard]] std::vector<vk::CommandBuffer>& GetCommandBuffers();
    private:
        vk::CommandPool m_CommandPool;

        std::vector<vk::CommandBuffer> m_DrawCmdbuffers;
        vk::Device* m_DeviceRef;
    };
} // namespace mortal
