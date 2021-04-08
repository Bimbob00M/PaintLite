#pragma once

#include <Windows.h>

#include <string>

namespace PaintLite
{
    extern const std::wstring APP_NAME;
    
    extern constexpr unsigned int UM_TOOLBAR_COMMAND{ WM_APP + 1 };
        
    enum EToolbarIDs
    {
        eTID_NEW = 20000, 
        eTID_LOAD, 
        eTID_SAVE,
        eTID_SAVE_AS, 
        eTID_SEP_1,
        eTID_PENCIL, 
        eTID_LINE, 
        eTID_RECT, 
        eTID_ELLIPSE, 
        eTID_ERASER,
        eTID_SEP_2, 
        eTID_PALETTE,
        eTID_MAIN_COLOR, 
        eTID_ADDITIONAL_COLOR,
        eTID_SEP_3, 
        eTID_THICK_1,
        eTID_THICK_2,
        eTID_THICK_3,
        eTID_THICK_4
    };   
}