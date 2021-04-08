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
            case UM_TOOLBAR_COMMAND:
            {
                auto id = LOWORD( wParam );
                switch( id )
                {
                    case eTID_LOAD:
                    {
                        SaveLoadManager slm( m_hWnd );
                        if( auto loadedBitmap = slm.load() )
                        {
                            if( Ok == loadedBitmap->GetLastStatus() )
                            {
                                m_saveState = false;
                                clear();
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

                        m_saveState = m_saveState ? slm.save( *m_canvas.get() ) : slm.saveAs( *m_canvas.get() );
                                                
                        if( !m_saveState )
                            MessageBeep( MB_ICONERROR );
                            
                        break;
                    }
                    case eTID_SAVE_AS:
                    {
                        SaveLoadManager slm( m_hWnd );

                        m_saveState =  slm.saveAs( *m_canvas.get() );

                        if( !m_saveState )
                            MessageBeep( MB_ICONERROR );
                        
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
                            if( eTID_ERASER == id  )
                                m_drawer.useAdditionalColor( true );
                            else
                                m_drawer.useAdditionalColor( false );
                            m_drawer.setTool( tool );
                        }
                        break;
                    }
                    case eTID_PALETTE:
                    {
                        Color newColor;
                        newColor.SetFromCOLORREF( static_cast<COLORREF>( lParam ) );
                        
                        if( HIWORD( wParam ) ) //if mainColor button checked                        
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
                        if( m_drawer.getTool() )
                        {
                            auto newThickness = m_drawer.getTool()->getPossibleThickness()[thicknessBtnId];
                            m_drawer.getTool()->setThickness( newThickness );
                        }
                        break;
                    }
                }
            }

            HANDLE_MSG( m_hWnd, WM_COMMAND, onCommand );
            HANDLE_MSG( m_hWnd, WM_PAINT, onPaint );
            HANDLE_MSG( m_hWnd, WM_SIZE, onSize );
            HANDLE_MSG( m_hWnd, WM_LBUTTONDOWN, onLBtnDown );
            HANDLE_MSG( m_hWnd, WM_LBUTTONUP, onLBtnUp );
            HANDLE_MSG( m_hWnd, WM_MOUSEMOVE, onMouseMove );
            HANDLE_MSG( m_hWnd, WM_RBUTTONDOWN, onRBtnDown );
            HANDLE_MSG( m_hWnd, WM_DESTROY, onDestroy );

            default:
                return DefWindowProc( m_hWnd, msg, wParam, lParam );
        }
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

    void CanvasWindow::onCommand( HWND hWnd, int id, HWND hwndCtl, UINT codeNotify )
    {
        switch( id )
        {
            case IDM_EXIT:
                DestroyWindow( m_hWnd );
                break;
        }
    }

    void CanvasWindow::onPaint( HWND hWnd )
    {
        PAINTSTRUCT ps;
        BeginPaint( hWnd, &ps );
        
        const int width = ps.rcPaint.right - ps.rcPaint.left;
        const int height = ps.rcPaint.bottom - ps.rcPaint.top;

        HDC hMemDC = CreateCompatibleDC( ps.hdc );
        HBITMAP hMemBitmap = CreateCompatibleBitmap( ps.hdc, width, height );
        HGDIOBJ oldBitmap = SelectObject( hMemDC, hMemBitmap );

        Graphics g( hMemDC );
        g.SetSmoothingMode( SmoothingModeHighQuality );
        g.DrawImage( m_canvas.get(), 0, 0 );

        if( m_drawer.isDrawing() )
            g.DrawImage( m_drawerCanvas.get(), 0, 0 );

        BitBlt( ps.hdc, 0, 0, width, height, hMemDC, 0, 0, SRCCOPY );

        SelectObject( hMemDC, oldBitmap );
        DeleteObject( hMemBitmap );
        DeleteDC( hMemDC );

        EndPaint( hWnd, &ps );
    }

    void CanvasWindow::onSize( HWND hWnd, UINT state, int width, int height )
    {
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
        DestroyWindow( hWnd );
    }
}