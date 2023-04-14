#pragma once
#include "Rendering/rendering_pass_base.h"
namespace mortal
{
    class TrianglePass : public RenderPassBase {
    public:
        explicit TrianglePass(RenderingSystemInfo& info);
        ~TrianglePass();

        virtual void Init() override {};
        virtual void ClearUp() override {};
        virtual void Draw() override;
    private:
        

    };
} // namespace mortal
