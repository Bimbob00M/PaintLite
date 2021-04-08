#pragma once

#include <Windows.h>
#include <gdiplus.h>

#include <string>

namespace PaintLite
{
    class SaveLoadManager
    {
    public:
        SaveLoadManager( HWND hWnd );
        ~SaveLoadManager();

        Gdiplus::Bitmap* load();
        bool saveAs( Gdiplus::Bitmap& source );
        bool save( Gdiplus::Bitmap& source );

    private:
        static WCHAR sm_fileName[300];
        HWND m_hWndOwner;

        void init( OPENFILENAME& ofn );
    };
}