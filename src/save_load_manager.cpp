#include "save_load_manager.h"

#include "gdiplus_helper_functions.h"

using namespace Gdiplus;

namespace PaintLite
{
    WCHAR SaveLoadManager::sm_fileName[300]{};

    SaveLoadManager::SaveLoadManager( HWND hWnd )
        : m_hWndOwner{ hWnd }
    {}

    SaveLoadManager::~SaveLoadManager()
    {
        m_hWndOwner = nullptr;
    }

    Bitmap* SaveLoadManager::load()
    {
        OPENFILENAME ofn{};
        init( ofn );
        ofn.lpstrFile[0] = '\0';

        if( GetOpenFileName( &ofn ) )
        {
            return Bitmap::FromFile( sm_fileName );
        }

        return false;
    }

    bool SaveLoadManager::saveAs( Bitmap& source )
    {
        OPENFILENAME sfn{};
        init( sfn );

        if( GetSaveFileName( &sfn ) == true )
        {
            CLSID bmpClsid;
            GetEncoderClsid( L"image/bmp", &bmpClsid );
            return Ok == source.Save( sfn.lpstrFile, &bmpClsid, NULL );
        }

        return false;
    }

    bool SaveLoadManager::save( Bitmap& source )
    {
        CLSID bmpClsid;
        GetEncoderClsid( L"image/bmp", &bmpClsid );
        return Ok == source.Save( sm_fileName, &bmpClsid, NULL );
    }

    void SaveLoadManager::init( OPENFILENAME& ofn )
    {
        ofn.lStructSize = sizeof( ofn );
        ofn.hwndOwner = m_hWndOwner;
        ofn.lpstrFile = sm_fileName;
        ofn.nMaxFile = sizeof( sm_fileName );
        ofn.lpstrFilter = L"All\0*.*\0Bitmap\0*.bmp\0";
        ofn.nFilterIndex = 2;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    }

}