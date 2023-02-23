#ifndef APPLICATION_H_
#define APPLICATION_H_

#include "Core/Core.h"
namespace mortal{

class MORTAL_API Application{
public:
    Application() = default;

    virtual void Run();
};

}//namespace mortal

#endif
