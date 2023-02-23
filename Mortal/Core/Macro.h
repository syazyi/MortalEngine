#ifndef MACRO_H
#define MACRO_H

#ifdef MORTAL_PLATFORM_WINDOWS
    #ifdef MORTAL_BUILD_DLL
        #define MORTAL_API __declspec(dllexport)
    #else
        #define MORTAL_API __declspec(dllimport)
    #endif
#else
    #define MORTAL_API
#endif


#endif