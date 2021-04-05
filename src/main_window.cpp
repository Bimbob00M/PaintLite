#include <string>


#include <Windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <gdiplus.h>

#include "src/constant.h"
#include "src/main_window.h"

using namespace Gdiplus;

namespace PaintLite
{
    static constexpr int ID_TOOLBAR{ 19999 };

    MainWindow::MainWindow() :
        BaseWindow{},
        m_winCanvas{ 960, 540 },
        m_drawerCanvas{ 960, 540, Color::Transparent },
        m_drawer{},
        m_tool{ Color::Black, 4 }
    {
        m_tool.getPen().SetLineCap( LineCapRound, LineCapRound, DashCapFlat );
        m_drawer.setTool( &m_tool );
    };

    LRESULT MainWindow::handleMessage( UINT msg, WPARAM wParam, LPARAM lParam )
    {
        switch( msg )
        {
            case WM_CREATE:
            {
                auto s = createToolbar( m_hWnd );
                break;
            }
            case WM_COMMAND:
            {
                switch( LOWORD( wParam ) )
                {
                    case IDM_EXIT:
                        DestroyWindow( m_hWnd );
                        break;
                    default:
                        return DefWindowProc( m_hWnd, msg, wParam, lParam );
                }
                break;
            }
            case WM_SIZE:
                return onSize( wParam, lParam );
            case WM_MOUSEMOVE:
            {
                if( m_drawer.isDrawing() )
                {
                    //SetFocus( m_hWnd );
                    POINT mousePos;
                    GetCursorPos( &mousePos );
                    ScreenToClient( m_hWnd, &mousePos );

                    Graphics drawerGraphics( m_drawerCanvas.get() );
                    m_drawer.draw( drawerGraphics, { mousePos.x, mousePos.y } );

                    InvalidateRect( m_hWnd, nullptr, false );
                }
                break;
            }
            case WM_LBUTTONDOWN:
            {
                POINT mousePos;
                GetCursorPos( &mousePos );
                ScreenToClient( m_hWnd, &mousePos );

                m_drawer.startDrawing( { mousePos.x, mousePos.y } );
                break;
            }
            case WM_LBUTTONUP:
            {
                POINT mousePos;
                GetCursorPos( &mousePos );
                ScreenToClient( m_hWnd, &mousePos );

                m_drawer.finishDrawing( { mousePos.x, mousePos.y } );

                Graphics g( m_winCanvas.get() );
                g.DrawImage( m_drawerCanvas.get(), 0, 0 );

                InvalidateRect( m_hWnd, nullptr, false );
                break;
            }
            case WM_ERASEBKGND:
                return 1;
            case WM_PAINT:
                return onPaint( wParam, lParam );
            case WM_CLOSE:
                return onClose( wParam, lParam );
            case WM_DESTROY:
                PostQuitMessage( 0 );
                break;
            default:
                return DefWindowProc( m_hWnd, msg, wParam, lParam );
        }
        return 0;
    }

    LRESULT MainWindow::onPaint( WPARAM /*wParam*/, LPARAM /*lParam*/ )
    {
        PAINTSTRUCT ps;
        HDC hWinDC = BeginPaint( m_hWnd, &ps );

        Graphics g( hWinDC );

        if( m_drawer.isDrawing() )
        {
            g.DrawImage( m_drawerCanvas.get(), 0, 0 );
        }
        else
        {
            g.DrawImage( m_winCanvas.get(), 0, 0 );
        }

        EndPaint( m_hWnd, &ps );
        return 0;
    }

    LRESULT MainWindow::onSize( WPARAM wParam, LPARAM lParam )
    {
        int width = LOWORD( lParam );
        int height = HIWORD( lParam );

        m_winCanvas.resize( width, height, true );
        m_drawerCanvas.resize( width, height );

        return 0;
    }

    LRESULT MainWindow::onClose( WPARAM /*wParam*/, LPARAM /*lParam*/ )
    {
        std::wstring mbText{ L"Are you sure you want to close this window?" };
        if( MessageBox( m_hWnd, mbText.c_str(), g_AppName.c_str(), MB_YESNO ) == IDYES )
            DestroyWindow( m_hWnd );

        return 0;
    }

    bool MainWindow::createToolbar( HWND hParent )
    {
        INITCOMMONCONTROLSEX iccx;
        iccx.dwSize = sizeof( INITCOMMONCONTROLSEX );
        iccx.dwICC = ICC_BAR_CLASSES;
        if( !InitCommonControlsEx( &iccx ) )
            return false;

        m_hToolbar = CreateWindowEx( 0,
                                     TOOLBARCLASSNAME,
                                     NULL,
                                     WS_CHILD | TBSTYLE_WRAPABLE,
                                     0,
                                     0,
                                     0,
                                     0,
                                     hParent,
                                     reinterpret_cast<HMENU>( ID_TOOLBAR ),
                                     GetModuleHandle( nullptr ),
                                     NULL );
        
        if( !m_hToolbar )
            return false;

        // If an application uses the CreateWindowEx function to create the 
        // toolbar, the application must send this message to the toolbar before 
        // sending the TB_ADDBITMAP or TB_ADDBUTTONS message. The CreateToolbarEx 
        // function automatically sends TB_BUTTONSTRUCTSIZE, and the size of the 
        // TBBUTTON structure is a parameter of the function.
        SendMessage( m_hToolbar, TB_BUTTONSTRUCTSIZE, ( WPARAM )sizeof( TBBUTTON ), 0 );


        TBADDBITMAP tbAddBmp = { 0 };
        tbAddBmp.hInst = HINST_COMMCTRL;
        tbAddBmp.nID = IDB_STD_SMALL_COLOR;

        SendMessage( m_hToolbar, TB_ADDBITMAP, 0, (WPARAM)& tbAddBmp );


        const int numButtons = 7;
        TBBUTTON tbButtons[numButtons] =
        {
            { MAKELONG( STD_FILENEW, 0 ), NULL, TBSTATE_ENABLED,
            BTNS_AUTOSIZE, {0}, 0, (INT_PTR)L"New" },
            { MAKELONG( STD_FILEOPEN, 0 ), NULL, TBSTATE_ENABLED,
            BTNS_AUTOSIZE, {0}, 0, (INT_PTR)L"Open" },
            { MAKELONG( STD_FILESAVE, 0 ), NULL, 0,
            BTNS_AUTOSIZE, {0}, 0, (INT_PTR)L"Save" },
            { MAKELONG( 0, 0 ), NULL, 0,
            TBSTYLE_SEP, {0}, 0, (INT_PTR)L"" }, // Separator
            { MAKELONG( STD_COPY, 0 ), NULL, TBSTATE_ENABLED,
            BTNS_AUTOSIZE, {0}, 0, (INT_PTR)L"Copy" },
            { MAKELONG( STD_CUT, 0 ), NULL, TBSTATE_ENABLED,
            BTNS_AUTOSIZE, {0}, 0, (INT_PTR)L"Cut" },
            { MAKELONG( STD_PASTE, 0 ), NULL, TBSTATE_ENABLED,
            BTNS_AUTOSIZE, {0}, 0, (INT_PTR)L"Paste" }
        };

        SendMessage( m_hToolbar, TB_ADDBUTTONS, numButtons, (LPARAM)tbButtons );

        SendMessage( m_hToolbar, TB_AUTOSIZE, 0, 0 );

        return TRUE;
    }

} //namespace PaintLite