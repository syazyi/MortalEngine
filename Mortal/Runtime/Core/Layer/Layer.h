#pragma once
#include "Core/Core.h"
#include "Events/Event.h"
namespace mortal
{
    class MORTAL_API Layer {
    public:
        explicit Layer(const char* name = "Layer") {};
        virtual ~Layer() {}

        virtual void OnAttach() {};
        virtual void OnDetch() {};
        virtual void OnEvent(Event& e) {};
        virtual void OnUpdate() {};

        inline const char* GetName() const {
                return m_debugName;
        }
    protected:
        const char* m_debugName;
    };


} // namespace mortal
