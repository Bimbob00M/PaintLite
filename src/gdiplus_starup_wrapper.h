#pragma once

#include <Windows.h>
#include <gdiplus.h>

namespace PaintLite
{
    //  This class is used to encapsulate the initialization 
    //  and deinitialization  of the Gdiplus module. It is recommended to 
    //  place an instance of the class on the stack
    //  at the beginning of the Main function.

    class GdiplusStarupWrapper
    {
    public:
        GdiplusStarupWrapper( ULONG_PTR token );
        ~GdiplusStarupWrapper();

    private:
        Gdiplus::GdiplusStartupInput startupInput;
        ULONG_PTR m_token;
    };
}