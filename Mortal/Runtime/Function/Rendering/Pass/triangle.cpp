#include "triangle.h"

namespace mortal
{
    TrianglePass::TrianglePass(RenderingSystemInfo& info) : RenderPassBase(info)
    {
        Init();
    }
    TrianglePass::~TrianglePass()
    {
        ClearUp();
    }

    void mortal::TrianglePass::Draw()
    {
        MORTAL_LOG_INFO("Triangle");
    }

} // namespace mortal
