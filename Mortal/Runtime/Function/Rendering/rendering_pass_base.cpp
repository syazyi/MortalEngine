#include "rendering_pass_base.h"

namespace mortal
{
	RenderPassBase::RenderPassBase(RenderingSystemInfo& info) : m_RenderingInfo(info)	
	{

	}

    std::vector<char> RenderPassBase::LoadShader(const std::string& fileName)
	{
        auto filePath = "../../Mortal/Shader/generated/spv/" + fileName + ".spv";
        std::ifstream file(filePath, std::ios::ate | std::ios::binary);

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file!");
        }

        size_t fileSize = (size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
	}
} // namespace mortal
