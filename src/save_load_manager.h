#pragma once

#include <Windows.h>
#include <gdiplus.h>

#include <string>

namespace PaintLite
{
    class SaveLoadManager
    {
    public:
        SaveLoadManager( HWND hWnd ) noexcept;
        ~SaveLoadManager() noexcept;

        Gdiplus::Bitmap* load() noexcept;
        bool saveAs( Gdiplus::Bitmap& source ) noexcept;
        bool save( Gdiplus::Bitmap& source ) noexcept;

        static void reset() noexcept { ms_fileName.clear(); }

    private:
        static std::wstring ms_fileName;
        HWND m_hWndOwner;

        void init( OPENFILENAME& ofn, LPWSTR str, UINT fileNameSize ) const noexcept;
        bool isContainFileExtension( const std::wstring& filename ) const noexcept;
    };
}