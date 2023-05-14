#include "Rendering/rendering_part_base.h"

namespace mortal
{
    class DeferredPart : public RenderPartBase{
    public:
        struct ModelUBO
        {
            glm::mat4 model;
            glm::mat4 view;
            glm::mat4 proj;
        };

        DeferredPart(RenderingSystemInfo& info);
        ~DeferredPart();
        virtual void Init() override;
        virtual void Draw() override;
        virtual void ClearUp() override;

    private:
        PrepareModelInfo m_ModelPlane;
        PrepareModelInfo m_ModelMonkeyHead;

        PrepareUniformInfo m_ModelUBO;
        ModelUBO m_ModelUBOData;
    };


} // namespace mortal
