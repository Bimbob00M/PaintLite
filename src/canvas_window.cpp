#include <Windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <gdiplus.h>

#include <algorithm>

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
        , m_drawerCanvas{ Color::Transparent }
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
            HANDLE_MSG( m_hWnd, WM_MOUSEWHEEL, onMouseWheel );
            HANDLE_MSG( m_hWnd, WM_RBUTTONDOWN, onRBtnDown );
            HANDLE_MSG( m_hWnd, WM_DESTROY, onDestroy );

            case UM_TOOLBAR_COMMAND:            
                onToolbarCommand( LOWORD( wParam ), static_cast<COLORREF>( lParam ) );
                break;

            case UM_CANVAS_SIZE:
                m_resizedWithOffsetRedraw = true;
                m_xDrawOffset = wParam;
                m_yDrawOffset = lParam;
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

    void CanvasWindow::initWndClass( WNDCLASSEX& outWndClass ) const
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

    void CanvasWindow::onToolbarCommand( int id, UINT codeNotify )
    {
        switch( id )
        {
            case eTID_NEW:
            {
                SaveLoadManager::reset();
                clear();
                resetScale();
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
                        resetScale();

                        if( auto hParent = GetParent( m_hWnd ) )
                        {
                            PostMessage( hParent, UM_CANVAS_SIZE, newWidth, newHeight );
                        }

                        Graphics g( m_canvas.get() );
                        g.DrawImage( loadedBitmap, 0, 0, newWidth, newHeight );
                        invalidateRect( nullptr );
                    }
                    delete loadedBitmap;
                }
                break;
            }
            case eTID_SAVE:
            {
                SaveLoadManager slm( m_hWnd );

                if( !m_saveState )
                {
                    m_saveState = slm.save( *m_canvas.get() );
                    if( !m_saveState )
                        MessageBeep( MB_ICONERROR );
                }
                else
                {
                    slm.save( *m_canvas.get() );
                }

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
                        m_drawer.setAdditionalColorFlag( true );
                    else
                        m_drawer.setAdditionalColorFlag( false );
                    m_drawer.setTool( tool );
                }
                break;
            }
            case eTID_MAIN_COLOR:
            case eTID_ADDITIONAL_COLOR:
            {
                Color newColor;
                newColor.SetFromCOLORREF( static_cast<COLORREF>( codeNotify ) );

                if( id == eTID_MAIN_COLOR )
                    m_drawer.setMainColor( newColor );
                else
                    m_drawer.setAdditionalColor( newColor );

                break;
            }
            case eTID_FILL:
            {
                m_drawer.setFillFlag( static_cast<bool>( codeNotify ));
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
        g.SetSmoothingMode( SmoothingModeAntiAlias );

        g.DrawImage( m_canvas.get(), 0, 0, m_canvas.get()->GetWidth(), m_canvas.get()->GetHeight() );

        if( m_drawer.isDrawing() )
            g.DrawImage( m_drawerCanvas.get(), 0, 0, m_drawerCanvas.get()->GetWidth(), m_drawerCanvas.get()->GetHeight() );
                
        if( m_memDC )
        {
            PRECT rect = &ps.rcPaint;
            BitBlt( ps.hdc, rect->left, rect->top , width, height, m_memDC, rect->left, rect->top, SRCCOPY );
        }

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

        if( !m_resizedWithOffsetRedraw )
        {
            m_canvas.resize( width, height );
        }
        else
        {
            m_canvas.resize( width, height, false, m_xDrawOffset, m_yDrawOffset );
            m_resizedWithOffsetRedraw = false;
        }
        m_drawerCanvas.resize( width, height );
    }

    void CanvasWindow::onMouseMove( HWND hWnd, int x, int y, UINT keyFlags )
    {
        if( auto hParent = GetParent( m_hWnd ) )
        {
            PostMessage( hParent, UM_MOUSE_POS_LABEL_UPDATE, x, y );
        }

        if( m_drawer.isDrawing() )
        {
            SetFocus( hWnd );

            Point mousePos{ x, y };

            m_drawer.trackShiftKeyState( ( keyFlags & MK_SHIFT ) == MK_SHIFT );

            Graphics drawerGraphics( m_drawerCanvas.get() );
            drawerGraphics.SetSmoothingMode( SmoothingModeAntiAlias );

            m_drawer.draw( drawerGraphics, mousePos );
            invalidateRect( nullptr );
        }
    }

    void CanvasWindow::onMouseWheel( HWND hWnd, int x, int y, int delta, UINT keyFlags )
    {
        if( ( keyFlags & MK_CONTROL ) == MK_CONTROL )
        {          

            if( delta > 0 )
            {
                if( m_scalingFactor != 1.F )
                    m_canvas.scale( 1 / m_scalingFactor );
                m_scalingFactor = ( std::min )( 1.F, m_scalingFactor * 2 );

            }
            else
            {
                m_scalingFactor = ( std::max )( 0.125F, m_scalingFactor / 2 );
                if( m_scalingFactor != 1.F )
                    m_canvas.scale( m_scalingFactor );
            }
 


            if( auto hParent = GetParent( m_hWnd ) )
            {
                PostMessage( hParent, UM_CANVAS_SCALE, static_cast<int>( m_scalingFactor * 100.F ), 0 );
            }

            invalidateRect( nullptr );
        }
    }

    void CanvasWindow::onLBtnDown( HWND hWnd, bool dblClick, int x, int y, UINT keyFlags )
    {
        SetCapture( hWnd );

        Point mousePos{ x, y };

        m_drawer.startDrawing( mousePos );
        invalidateRect( nullptr );
    }

    void CanvasWindow::onLBtnUp( HWND hWnd, int x, int y, UINT keyFlags )
    {
        ReleaseCapture();
        
        if( !m_drawer.isCanceled() )
        {
            Point mousePos{ x, y };

            Graphics g( m_canvas.get() );
            g.SetSmoothingMode( SmoothingModeAntiAlias );

            g.DrawImage( m_drawerCanvas.get(), 
                         0, 0, 
                         m_drawerCanvas.get()->GetWidth(), 
                         m_drawerCanvas.get()->GetHeight() );

            m_drawer.finishDrawing( mousePos );
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
    void CanvasWindow::resetScale() noexcept
    {
        if( auto hParent = GetParent( m_hWnd ) )
        {
            PostMessage( hParent, UM_CANVAS_SCALE, static_cast<int>( m_scalingFactor * 100.F ), 0 );
        }
    }
}