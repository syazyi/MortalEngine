#pragma once 
#include "Rendering/rendering_part_base.h"
namespace mortal
{

    class BlingPhong : public RenderPartBase{
    public:
        BlingPhong();
        ~BlingPhong();

        virtual void Init() override;
        virtual void ClearUp() override;
        virtual void Draw() override;

    private:
        
    };
    
} // namespace mortal
