#pragma once
#include "Layer/Layer.h"
namespace mortal
{
    class MORTAL_API LayerStack {
    public:
        LayerStack();
        ~LayerStack();

        void PushLayer(Layer* layer);
        void PushOverlay(Layer* layer);
        void PopLayer(Layer* layer);
        void PopOverlay(Layer* layer);

        auto begin() {
            return m_Layers.begin();
        }
        auto end() {
            return m_Layers.end();
        }

    private:
        std::vector<Layer*> m_Layers;
        std::vector<Layer*>::iterator m_LayerInsert;
    };


} // namespace mortal
