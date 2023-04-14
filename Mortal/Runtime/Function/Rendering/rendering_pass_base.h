#pragma once
#include "rendering.h"

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
    protected:
        RenderingSystemInfo& m_RenderingInfo;
    };
} // namespace mortal
