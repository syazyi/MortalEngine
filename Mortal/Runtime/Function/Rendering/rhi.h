#pragma once

namespace mortal
{
    class RHI{
    public:
        virtual ~RHI();
        virtual void Init() = 0;
        //virtual void PrepareContext() = 0;
    };


} // namespace mortal
