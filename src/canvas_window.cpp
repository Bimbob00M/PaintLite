#include <Windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <gdiplus.h>

#include "base_window.hpp"
#include "canvas_window.h"
#include "global.h"
#include "gdiplus_helper_functions.h"
#include "save_load_manager.h"

using namespace Gdiplus;

namespace PaintLite
{
    CanvasWindow::CanvasWindow()
        : BaseWindow{}
        , m_canvas{}
        , m_drawerCanvas{ Color::Transparent }
        , m_drawer{}
        , m_toolkit{}
        , m_memDC{}
        , m_memBitmap{}
    {
        if( auto tool = m_toolkit.getTool( eTID_PENCIL ) )
        {
            m_drawer.setTool( tool );
        }
    };

    LRESULT CanvasWindow::handleMessage( UINT msg, WPARAM wParam, LPARAM lParam )
    {
        switch( msg )
        {
            HANDLE_MSG( m_hWnd, WM_CREATE, onCreate );
            HANDLE_MSG( m_hWnd, WM_COMMAND, onCommand );
            HANDLE_MSG( m_hWnd, WM_PAINT, onPaint );
            HANDLE_MSG( m_hWnd, WM_SIZE, onSize );
            HANDLE_MSG( m_hWnd, WM_LBUTTONDOWN, onLBtnDown );
            HANDLE_MSG( m_hWnd, WM_LBUTTONUP, onLBtnUp );
            HANDLE_MSG( m_hWnd, WM_MOUSEMOVE, onMouseMove );
            HANDLE_MSG( m_hWnd, WM_RBUTTONDOWN, onRBtnDown );
            HANDLE_MSG( m_hWnd, WM_DESTROY, onDestroy );

            case UM_TOOLBAR_COMMAND:            
                onToolbarCommand( LOWORD( wParam ), static_cast<COLORREF>( lParam ) );
                break;
            
            default:
                return DefWindowProc( m_hWnd, msg, wParam, lParam );
        }

        return 0L;
    }

    LRESULT CanvasWindow::clear() noexcept
    {
        bool main = !m_canvas.clear();
        bool additional = !m_drawerCanvas.clear( Color::Transparent );
        return main && additional;
    }

    void CanvasWindow::initWndClass( WNDCLASS& outWndClass ) const
{
        BaseWindow::initWndClass( outWndClass );

        outWndClass.hCursor = LoadCursor( NULL, IDC_CROSS );        
    }

    bool CanvasWindow::onCreate( HWND hWnd, LPCREATESTRUCT lpCreateStruct )
    {
        if( HDC hWinDC = GetDC( m_hWnd ) )
        {
            if( m_memDC = CreateCompatibleDC( hWinDC ) )
            {
                RECT clientRect;
                GetClientRect( m_hWnd, &clientRect );
                if( m_memBitmap = CreateCompatibleBitmap( hWinDC, clientRect.right, clientRect.bottom) )
                {
                    DeleteObject( SelectObject( m_memDC, m_memBitmap ) );
                    ReleaseDC( m_hWnd, hWinDC );
                    return true;
                }
                ReleaseDC( m_hWnd, hWinDC );
            }
        }

        return false;
    }

    void CanvasWindow::onCommand( HWND hWnd, int id, HWND hwndCtl, UINT codeNotify )
    {
        switch( id )
        {
            case IDM_EXIT:
                DestroyWindow( m_hWnd );
                break;
        }
    }

    void CanvasWindow::onToolbarCommand( int id, COLORREF color )
    {
        switch( id )
        {
            case eTID_NEW:
            {
                SaveLoadManager::reset();
                clear();
                invalidateRect( nullptr );

                break;
            }
            case eTID_LOAD:
            {
                SaveLoadManager slm( m_hWnd );
                if( auto loadedBitmap = slm.load() )
                {
                    if( Ok == loadedBitmap->GetLastStatus() )
                    {
                        m_saveState = false;

                        auto newWidth = loadedBitmap->GetWidth();
                        auto newHeight = loadedBitmap->GetHeight();
                        m_canvas.resize( newWidth, newHeight );
                        m_drawerCanvas.resize( newWidth, newHeight );
                        clear();
                        SetWindowPos( m_hWnd, nullptr, 0, 0, newWidth, newHeight, SWP_NOMOVE );

                        if( auto hParent = GetParent( m_hWnd ) )
                        {
                            SendMessage( hParent, UM_CANVAS_SIZE, newWidth, newHeight );
                        }

                        Graphics g( m_canvas.get() );
                        g.DrawImage( loadedBitmap, 0, 0 );
                        invalidateRect( nullptr );
                    }
                    delete loadedBitmap;
                }
                break;
            }
            case eTID_SAVE:
            {
                SaveLoadManager slm( m_hWnd );

                //m_saveState = m_saveState ? slm.save( *m_canvas.get() ) : slm.saveAs( *m_canvas.get() );
                m_saveState = slm.save( *m_canvas.get() );

                if( !m_saveState )
                    MessageBeep( MB_ICONERROR );

                break;
            }
            case eTID_SAVE_AS:
            {
                SaveLoadManager slm( m_hWnd );

                if( !m_saveState )
                {
                    m_saveState = slm.saveAs( *m_canvas.get() );
                    if( !m_saveState )
                        MessageBeep( MB_ICONERROR );
                }
                else
                {
                    slm.saveAs( *m_canvas.get() );
                }

                break;
            }
            case eTID_PENCIL:
            case eTID_LINE:
            case eTID_RECT:
            case eTID_ELLIPSE:
            case eTID_ERASER:
            {
                if( auto tool = m_toolkit.getTool( static_cast<EToolbarIDs>( id ) ) )
                {
                    if( eTID_ERASER == id )
                        m_drawer.useAdditionalColor( true );
                    else
                        m_drawer.useAdditionalColor( false );
                    m_drawer.setTool( tool );
                }
                break;
            }
            case eTID_MAIN_COLOR:
            case eTID_ADDITIONAL_COLOR:
            {
                Color newColor;
                newColor.SetFromCOLORREF( color );

                if( id == eTID_MAIN_COLOR )
                    m_drawer.setMainColor( newColor );
                else
                    m_drawer.setAdditionalColor( newColor );

                break;
            }
            case eTID_THICK_1:
            case eTID_THICK_2:
            case eTID_THICK_3:
            case eTID_THICK_4:
            {
                const auto thicknessBtnId = id - eTID_THICK_1;
                if( auto tool = m_drawer.getTool() )
                {
                    auto newThickness = tool->getPossibleThickness()[thicknessBtnId];
                    tool->setThickness( static_cast<REAL>( newThickness ) );
                }
                break;
            }
        }
    }

    void CanvasWindow::onPaint( HWND hWnd )
    {
        PAINTSTRUCT ps;
        BeginPaint( hWnd, &ps );

        const int width = ps.rcPaint.right - ps.rcPaint.left;
        const int height = ps.rcPaint.bottom - ps.rcPaint.top;
        
        Graphics g( m_memDC ? m_memDC : ps.hdc );

        g.SetSmoothingMode( SmoothingModeHighQuality );
        g.DrawImage( m_canvas.get(), 0, 0 );

        if( m_drawer.isDrawing() )
            g.DrawImage( m_drawerCanvas.get(), 0, 0 );

        if( m_memDC )
            BitBlt( ps.hdc, 0, 0, width, height, m_memDC, 0, 0, SRCCOPY );

        EndPaint( hWnd, &ps );
    }

    void CanvasWindow::onSize( HWND hWnd, UINT state, int width, int height )
    {
        if( auto hWinDC = GetDC( m_hWnd ) )
        {
            DeleteObject( m_memBitmap );
            m_memBitmap = CreateCompatibleBitmap( m_memDC, width, height );
            DeleteObject( SelectObject( m_memDC, m_memBitmap ) );
            ReleaseDC( m_hWnd, hWinDC );
        }
        m_canvas.resize( width, height, true );
        m_drawerCanvas.resize( width, height );
    }

    void CanvasWindow::onMouseMove( HWND hWnd, int x, int y, UINT keyFlags )
    {
        if( m_drawer.isDrawing() )
        {
            SetFocus( hWnd );
            POINT mousePos;
            GetCursorPos( &mousePos );
            ScreenToClient( hWnd, &mousePos );

            RECT cr;
            GetClientRect( hWnd, &cr );

            m_drawer.trackShiftKeyState( keyFlags & MK_SHIFT || keyFlags & MK_CONTROL );

            Graphics drawerGraphics( m_drawerCanvas.get() );
            m_drawer.draw( drawerGraphics, { mousePos.x, mousePos.y } );
            invalidateRect( nullptr );
        }
    }

    void CanvasWindow::onLBtnDown( HWND hWnd, bool dblClick, int x, int y, UINT keyFlags )
    {
        SetCapture( hWnd );

        POINT mousePos;
        GetCursorPos( &mousePos );
        ScreenToClient( hWnd, &mousePos );

        m_drawer.startDrawing( { mousePos.x, mousePos.y } );
        invalidateRect( nullptr );
    }

    void CanvasWindow::onLBtnUp( HWND hWnd, int x, int y, UINT keyFlags )
    {
        ReleaseCapture();

        POINT mousePos;
        GetCursorPos( &mousePos );
        ScreenToClient( hWnd, &mousePos );

        if( !m_drawer.isCanceled() )
        {
            Graphics g( m_canvas.get() );
            g.DrawImage( m_drawerCanvas.get(), 0, 0 );
            m_drawer.finishDrawing( { mousePos.x, mousePos.y } );
            m_drawerCanvas.clear( Color::Transparent );
            invalidateRect( nullptr );

            if( isSaved() )
                m_saveState = false;
        }
    }

    void CanvasWindow::onRBtnDown( HWND hWnd, bool dblClick, int x, int y, UINT keyFlags )
    {
        m_drawer.cancelDrawing();
        m_drawerCanvas.clear( Color::Transparent );
        invalidateRect( nullptr );
    }

    void CanvasWindow::onDestroy( HWND hWnd )
    {
        DeleteObject( m_memDC );
        DeleteObject( m_memBitmap );
        DestroyWindow( hWnd );
    }
}