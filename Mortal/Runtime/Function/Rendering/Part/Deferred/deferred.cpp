#include "deferred.h"

namespace mortal
{
    DeferredPart::DeferredPart(RenderingSystemInfo& info) : RenderPartBase(info)
    {
        Init();
    }

    DeferredPart::~DeferredPart()
    {
        ClearUp();
    }

    void DeferredPart::Init()
    {
        auto& device = m_RenderingInfo.device.GetDevice();
        auto extent2D = m_RenderingInfo.window.GetExtent2D();
        {
            m_ModelPlane = PrepareModel("../../Asset/Model/Plane.obj");
            m_ModelMonkeyHead = PrepareModel("../../Asset/Model/MonkeyHeadFaceXAxis.obj");
            m_ModelUBOData.model = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            m_ModelUBOData.proj = glm::perspective(glm::radians(45.f), (float)extent2D.width / (float)extent2D.height, 0.1f, 100.f);
            m_ModelUBOData.proj[1][1] *= -1.f;



        }

    }

    void DeferredPart::Draw()
    {
        auto& device = m_RenderingInfo.device.GetDevice();

    }

    void DeferredPart::ClearUp()
    {
        auto& device = m_RenderingInfo.device.GetDevice();
        device.waitIdle();

        ClearUpPrepareUniform(m_ModelUBO);
        ClearUpPrepareModel(m_ModelMonkeyHead);
        ClearUpPrepareModel(m_ModelPlane);
    }

} // namespace mortal
