#pragma once

#include <Windows.h>

#include <string>

namespace PaintLite
{
    extern const std::wstring APP_NAME;

    constexpr unsigned int UM_TOOLBAR_COMMAND{ WM_APP + 1 };
    constexpr unsigned int UM_CANVAS_SIZE{ WM_APP + 2 };
    constexpr unsigned int UM_CANVAS_SCALE{ WM_APP + 3 };
    constexpr unsigned int UM_MOUSE_POS_LABEL_UPDATE{ WM_APP + 4 };

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
        eTID_MAIN_COLOR, 
        eTID_ADDITIONAL_COLOR,
        eTID_FILL,
        eTID_SEP_3, 
        eTID_THICK_1,
        eTID_THICK_2,
        eTID_THICK_3,
        eTID_THICK_4
    };   
}