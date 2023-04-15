#pragma once
#include <string>
#include "rendering.h"
#include "rendering_system.h"
namespace mortal
{
    struct RenderingSystemInfo;
    class RenderPassBase {
    public:
        RenderPassBase(RenderingSystemInfo& info);
        virtual ~RenderPassBase() {}

        virtual void Init() = 0;
        virtual void ClearUp() = 0;
        virtual void Draw() = 0;
        std::vector<char> LoadShader(const std::string& fileName);
    protected:
        RenderingSystemInfo& m_RenderingInfo;
    };
} // namespace mortal
