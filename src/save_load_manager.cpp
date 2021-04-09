#include <algorithm>
#include <cwctype>
#include <string>

#include "save_load_manager.h"
#include "gdiplus_helper_functions.h"

using namespace Gdiplus;

namespace PaintLite
{
    std::wstring SaveLoadManager::ms_fileName{};

    SaveLoadManager::SaveLoadManager( HWND hWnd ) noexcept
        : m_hWndOwner{ hWnd }
    {}

    SaveLoadManager::~SaveLoadManager() noexcept
    {
        m_hWndOwner = nullptr;
    }

    Bitmap* SaveLoadManager::load() noexcept
    {
        OPENFILENAME ofn{};
        WCHAR filename[300]{};
        init( ofn, filename, sizeof( filename ) );
        ofn.lpstrFilter = L"All\0*.*\0Bitmap\0*.bmp\0";
        ofn.nFilterIndex = 2;

        if( GetOpenFileName( &ofn ) )
        {
            ms_fileName = ofn.lpstrFile;
            return Bitmap::FromFile( ms_fileName.c_str() );
        }

        return nullptr;
    }

    bool SaveLoadManager::saveAs( Bitmap& source ) noexcept
    {
        OPENFILENAME sfn{};
        WCHAR filename[300]{};
        init( sfn, filename, sizeof( filename ) );

        if( GetSaveFileName( &sfn ) )
        {
            ms_fileName = sfn.lpstrFile;

            if( !isContainFileExtension( ms_fileName ) )
            {
                ms_fileName += L".bmp";
            }

            return save( source );
        }

        return false;
    }

    bool SaveLoadManager::save( Bitmap& source ) noexcept
    {
        if( ms_fileName.empty() || !isContainFileExtension( ms_fileName ) )
        {
            return saveAs( source );
        }
        else
        {
            CLSID bmpClsid;
            GetEncoderClsid( L"image/bmp", &bmpClsid );
            return Ok == source.Save( ms_fileName.c_str(), &bmpClsid, NULL );
        }
    }

    void SaveLoadManager::init( OPENFILENAME& ofn, LPWSTR filename, UINT fileNameSize ) const noexcept
    {
        ofn.lStructSize = sizeof( ofn );
        ofn.hwndOwner = m_hWndOwner;
        ofn.lpstrFile = filename;
        ofn.nMaxFile = fileNameSize;
        ofn.lpstrFilter = L"Bitmap\0*.bmp\0";
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = NULL;
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    }

    bool SaveLoadManager::isContainFileExtension( const std::wstring& filename ) const noexcept
    {
        auto dotBeforeExtentionPos = filename.find_last_of( L"." );
        if( dotBeforeExtentionPos != std::wstring::npos )
        {
            auto str_tolower = []( std::wstring s )
            {
                std::transform( s.begin(), s.end(), s.begin(),
                                []( WCHAR c ){ return std::towlower( c ); }
                );
                return s;
            };

            if( str_tolower( ( filename.substr( dotBeforeExtentionPos + 1 ) ) ) != L"bmp" )
            {
                return false;
            }
            else
            {
                return true;
            }
        }

        return false;
    }
}