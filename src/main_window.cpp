#include <Windows.h>
#include <windowsx.h>
#include <shlwapi.h>
#include <commctrl.h>
#include <gdiplus.h>

#include <algorithm>
#include <string>

#include "drawing_tool.h"
#include "global.h"
#include "main_window.h"
#include "resource.h"

using namespace Gdiplus;

namespace PaintLite
{
    static constexpr int ID_TOOLBAR{ 10001 };
    static constexpr int ID_CANVAS_WIN{ 10002 };
    static constexpr int ID_STATUSBAR{ 10003 };
    
    static constexpr int s_squareSideSize = 6; // the size of the side of the resizing square of the drawing area
    static constexpr RECT s_canvasWinOffset{ 5 + s_squareSideSize,
                                             5 + s_squareSideSize,
                                             10 + s_squareSideSize,
                                             10 + s_squareSideSize };
    
    static int s_horzScrollbarThickness;
    static int s_vertScrollbarThickness;
            
    LRESULT MainWindow::handleMessage( UINT msg, WPARAM wParam, LPARAM lParam )
    {
        switch( msg )
        {            
            HANDLE_MSG( m_hWnd, WM_CREATE, onCreate );
            HANDLE_MSG( m_hWnd, WM_COMMAND, onCommand );
            HANDLE_MSG( m_hWnd, WM_PAINT, onPaint );
            HANDLE_MSG( m_hWnd, WM_SIZE, onSize );
            HANDLE_MSG( m_hWnd, WM_HSCROLL, onHScroll );
            HANDLE_MSG( m_hWnd, WM_VSCROLL, onVScroll );
            HANDLE_MSG( m_hWnd, WM_LBUTTONDOWN, onLBtnDown );
            HANDLE_MSG( m_hWnd, WM_LBUTTONUP, onLBtnUp );
            HANDLE_MSG( m_hWnd, WM_RBUTTONDOWN, onRBtnDown );
            HANDLE_MSG( m_hWnd, WM_MOUSEMOVE, onMouseMove );
            HANDLE_MSG( m_hWnd, WM_CLOSE, onClose );
            HANDLE_MSG( m_hWnd, WM_DESTROY, onDestroy );
                       
            case UM_CANVAS_SIZE:
                onCanvasSize( wParam, lParam );
                break;
            
            case UM_CANVAS_SCALE:
                updateScaleLabel( static_cast<int>( wParam ) );
                break;
            
            case UM_MOUSE_POS_LABEL_UPDATE:
                updateMousePosLabel( wParam, lParam );
                break;
                        
            default:
                return DefWindowProc( m_hWnd, msg, wParam, lParam );
        }
        return 0;
    }

    void MainWindow::initWndClass( WNDCLASSEX& outWndClass ) const
    {
        BaseWindow::initWndClass( outWndClass );

        outWndClass.hbrBackground = GetStockBrush( LTGRAY_BRUSH );
        outWndClass.hIcon = LoadIcon( GetModuleHandle( nullptr ), MAKEINTRESOURCE( IDI_PAINTLITE ) );
        outWndClass.hCursor = LoadCursor( nullptr, IDC_ARROW );
    }

    bool MainWindow::onCreate( HWND hWnd, LPCREATESTRUCT lpCreateStruct )
    {
        if( !createToolbar() || !createStatusbar() )
            return false;

        if( HDC hWinDC = GetDC( m_hWnd ) )
        {
            if( m_memDC = CreateCompatibleDC( hWinDC ) )
            {
                RECT clientRect;
                GetClientRect( m_hWnd, &clientRect );
                if( m_memBitmap = CreateCompatibleBitmap( hWinDC, clientRect.right, clientRect.bottom ) )
                {
                    DeleteObject( SelectObject( m_memDC, m_memBitmap ) );
                }
                ReleaseDC( m_hWnd, hWinDC );
            }
        }

        auto style = GetWindowLong( m_hWnd, GWL_STYLE );
        s_horzScrollbarThickness = ( ( ~style & WS_HSCROLL ) == WS_HSCROLL ) ? 15 : 0;
        s_vertScrollbarThickness = ( ( ~style & WS_VSCROLL ) == WS_VSCROLL ) ? 15 : 0;

        RECT workRect;
        getWorkRect( workRect );
        m_canvasRect.Width = workRect.right - s_canvasWinOffset.right - s_horzScrollbarThickness;
        m_canvasRect.Height = workRect.bottom - workRect.top - s_canvasWinOffset.bottom - s_vertScrollbarThickness;
        calculateÑanvasPos( workRect );

        updateMousePosLabel( -1, -1 );
        updateCanvasSizeLabel();
        updateScaleLabel( 100 );

        auto result = m_canvasWin.create( nullptr,
                                          WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS,
                                          0,
                                          0,
                                          0,
                                          m_canvasRect.Width,
                                          m_canvasRect.Height,
                                          m_hWnd,
                                          reinterpret_cast<HMENU>( ID_CANVAS_WIN ) );

        m_horzScroll = { 0, 0, 0 };
        m_vertScroll = { 0, 0, 0 };

        return result;
    }

    void MainWindow::onCommand( HWND hWnd, int id, HWND hwndCtl, UINT codeNotify )
    {
        switch( id )
        {
            case eTID_PENCIL:
            case eTID_LINE:
            case eTID_RECT:
            case eTID_ELLIPSE:
            case eTID_ERASER:
            {
                // set the thickness selection button to the checked state
                // depending on the current thickness of the active tool

                const auto activeTool = m_canvasWin.getToolkit().getTool( static_cast<EToolbarIDs>( id ) );
                const auto& possibleThickness = activeTool->getPossibleThickness();
                const auto thicknessModeIndex = std::find( possibleThickness.cbegin(), 
                                                           possibleThickness.cend(), 
                                                           activeTool->getThickness() );
                
                if( thicknessModeIndex != possibleThickness.cend() )
                {
                    auto targetThicknessBtnIndex = eTID_THICK_1 + std::distance( possibleThickness.cbegin(),
                                                                                 thicknessModeIndex );

                    PostMessage( m_toolbar, TB_CHECKBUTTON, targetThicknessBtnIndex, true );
                }                
            }
            case eTID_NEW:
            case eTID_LOAD:
            case eTID_SAVE:
            case eTID_SAVE_AS:
            case eTID_THICK_1:
            case eTID_THICK_2:
            case eTID_THICK_3:
            case eTID_THICK_4:
            {
                PostMessage( m_canvasWin.getWindowHandle(), UM_TOOLBAR_COMMAND, id, 0 );
                break;
            }
            case eTID_MAIN_COLOR:
            case eTID_ADDITIONAL_COLOR:
            {
                CHOOSECOLOR cc{};                     // common dialog box structure 
                static COLORREF s_customColors[16];   // array of custom colors by default black
                static COLORREF s_rgbCurrent;              // initial color selection

                cc.lStructSize = sizeof( cc );
                cc.hwndOwner = m_hWnd;
                cc.lpCustColors = static_cast<LPDWORD>( s_customColors );
                cc.rgbResult = s_rgbCurrent;
                cc.Flags = CC_FULLOPEN | CC_RGBINIT;

                if( ChooseColor( &cc ) == TRUE )
                {
                    s_rgbCurrent = cc.rgbResult;

                    const int colorBtnIndex = id - eTID_NEW;
                    constexpr int numSeparators = 2; // separators before target button

                    HBITMAP newIcon = nullptr;
                    Bitmap* bitmap = createColorBitmap( s_rgbCurrent, 48, 48 );
                    bitmap->GetHBITMAP( {}, &newIcon );
                    ImageList_Replace( m_toolbarIcons, colorBtnIndex - numSeparators, newIcon, 0 );
                    DeleteObject( newIcon );
                    delete bitmap;

                    PostMessage( m_toolbar, TB_CHANGEBITMAP, id, colorBtnIndex - numSeparators );
                    PostMessage( m_canvasWin.getWindowHandle(), UM_TOOLBAR_COMMAND, id, static_cast<LPARAM>( s_rgbCurrent ) );
                }
                break;
            }
            case eTID_FILL:
            {
                auto isChecked = SendMessage( m_toolbar, TB_ISBUTTONCHECKED, eTID_FILL, 0 );
                PostMessage( m_canvasWin.getWindowHandle(), UM_TOOLBAR_COMMAND, id, isChecked );
                break;
            }
            case IDM_EXIT:
                DestroyWindow( m_hWnd );
                break;
        }
    }
    
    void MainWindow::onPaint( HWND hWnd )
    {
        PAINTSTRUCT ps;
        BeginPaint( hWnd, &ps );
            
        const int width = ps.rcPaint.right - ps.rcPaint.left;
        const int height = ps.rcPaint.bottom - ps.rcPaint.top;

        auto dc = m_memDC ? m_memDC : ps.hdc;

        HBRUSH bkgnd = GetStockBrush( LTGRAY_BRUSH );
        FillRect( dc, &ps.rcPaint, bkgnd );
        DeleteObject( bkgnd );

        Graphics g( dc );
        SolidBrush shadowBrush( Color( 15, 0, 0, 0 ) );
        g.FillRectangle( &shadowBrush, m_canvasRect.X + 7, m_canvasRect.Y + 7, m_canvasRect.Width, m_canvasRect.Height );

        drawResizingBoxes( &g );

        if( m_resizeCanvasFlag )
        {
            g.DrawImage( m_canvasWin.getCanvas()->get(), m_canvasRect );
            drawResizeArea( &g );
        }

        if( m_memDC )        
            BitBlt( ps.hdc, 0, 0, width, height, m_memDC, 0, 0, SRCCOPY );

        EndPaint( hWnd, &ps );
    }

    void MainWindow::onSize( HWND hWnd, UINT state, int width, int height )
    {  
        if( width < 1 || height < 1 )
            return;

        if( auto hWinDC = GetDC( m_hWnd ) )
        {
            DeleteObject( m_memBitmap );
            m_memBitmap = CreateCompatibleBitmap( m_memDC, width, height );
            DeleteObject( SelectObject( m_memDC, m_memBitmap ) );
            ReleaseDC( m_hWnd, hWinDC );
        }

        SendMessage( m_toolbar, TB_AUTOSIZE, state, MAKELPARAM( width, height ) );
        SendMessage( m_statusbar, WM_SIZE, width, height );

        resizeScrollbars( width, height );

        RECT workArea;
        getWorkRect( workArea );
        calculateÑanvasPos( workArea );
        
        SetWindowPos( m_canvasWin.getWindowHandle(),
                      nullptr,
                      m_canvasRect.X - m_horzScroll.current,
                      m_canvasRect.Y - m_vertScroll.current,
                      0, 0,
                      SWP_NOSIZE | SWP_NOZORDER );

        calculateResizingBoxesPos();
    }

    void MainWindow::onHScroll( HWND hWnd, HWND hwndCtl, UINT code, int pos )
    {
        int xDelta;     // xDelta = new_pos - current_pos  
        int xNewPos;    // new position 
        int yDelta = 0;

        switch( code )
        {
            // User clicked the scroll bar shaft left of the scroll box. 
            case SB_PAGEUP:
                xNewPos = m_horzScroll.current - 50;
                break;

                // User clicked the scroll bar shaft right of the scroll box. 
            case SB_PAGEDOWN:
                xNewPos = m_horzScroll.current + 50;
                break;

                // User clicked the left arrow. 
            case SB_LINEUP:
                xNewPos = m_horzScroll.current - 5;
                break;

                // User clicked the right arrow. 
            case SB_LINEDOWN:
                xNewPos = m_horzScroll.current + 5;
                break;

                // User dragged the scroll box. 
            case SB_THUMBPOSITION:
                xNewPos = pos;
                break;

            default:
                xNewPos = m_horzScroll.current;
        }

        // New position must be between 0 and the screen width. 
        xNewPos = ( std::max )( 0, xNewPos );
        xNewPos = ( std::min )( m_horzScroll.max, xNewPos );

        // If the current position does not change, do not scroll.
        if( xNewPos == m_horzScroll.current )
            return;

        // Determine the amount scrolled (in pixels). 
        xDelta = xNewPos - m_horzScroll.current;

        // Reset the current scroll position. 
        m_horzScroll.current = xNewPos;

        SetWindowPos( m_canvasWin.getWindowHandle(),
                      nullptr,
                      m_canvasRect.X -= xDelta,
                      m_canvasRect.Y,
                      0, 0,
                      SWP_NOSIZE | SWP_NOZORDER );

        calculateResizingBoxesPos();
                
        RECT workArea;
        getWorkRect( workArea );
        invalidateRect( &workArea );

        // Reset the scroll bar
        SCROLLINFO si;
        si.cbSize = sizeof( si );
        si.fMask = SIF_POS;
        GetScrollInfo( m_hWnd, SB_HORZ, &si );
        si.nPos = m_horzScroll.current;
        SetScrollInfo( m_hWnd, SB_HORZ, &si, true);
    }

    void MainWindow::onVScroll( HWND hWnd, HWND hwndCtl, UINT code, int pos )
    {
        int xDelta = 0;
        int yDelta;     // yDelta = new_pos - current_pos 
        int yNewPos;    // new position 

        switch( code )
        {
            // User clicked the scroll bar shaft above the scroll box. 
            case SB_PAGEUP:
                yNewPos = m_vertScroll.current - 50;
                break;

                // User clicked the scroll bar shaft below the scroll box. 
            case SB_PAGEDOWN:
                yNewPos = m_vertScroll.current + 50;
                break;

                // User clicked the top arrow. 
            case SB_LINEUP:
                yNewPos = m_vertScroll.current - 5;
                break;

                // User clicked the bottom arrow. 
            case SB_LINEDOWN:
                yNewPos = m_vertScroll.current + 5;
                break;

                // User dragged the scroll box. 
            case SB_THUMBPOSITION:
                yNewPos = pos;
                break;

            default:
                yNewPos = m_vertScroll.current;
        }

        // New position must be between 0 and the screen height. 
        yNewPos = ( std::max )( 0, yNewPos );
        yNewPos = ( std::min )( m_vertScroll.max, yNewPos );

        // If the current position does not change, do not scroll.
        if( yNewPos == m_vertScroll.current )
            return;
        
        // Determine the amount scrolled (in pixels). 
        yDelta = yNewPos - m_vertScroll.current;

        // Reset the current scroll position. 
        m_vertScroll.current = yNewPos;

        SetWindowPos( m_canvasWin.getWindowHandle(),
                      nullptr,
                      m_canvasRect.X,
                      m_canvasRect.Y -= yDelta,
                      0, 0,
                      SWP_NOSIZE | SWP_NOZORDER );
        
        calculateResizingBoxesPos();

        RECT workArea;
        getWorkRect( workArea );
        workArea.top -= 1; // toolbar border
        invalidateRect( &workArea );

        // Reset the scroll bar. 
        SCROLLINFO si;
        si.cbSize = sizeof( si );
        si.fMask = SIF_POS;
        GetScrollInfo( m_hWnd, SB_VERT, &si );
        si.nPos = m_vertScroll.current;
        SetScrollInfo( m_hWnd, SB_VERT, &si, true );
    }

    void MainWindow::onCanvasSize( int width, int height )
    {
        m_canvasRect.Width = width;
        m_canvasRect.Height = height;

        RECT winRect;
        GetWindowRect( m_hWnd, &winRect );
        resizeScrollbars( winRect.right - winRect.left, winRect.bottom - winRect.top );

        RECT workArea;
        getWorkRect( workArea );
        calculateÑanvasPos( workArea );

        updateCanvasSizeLabel();

        SetWindowPos( m_canvasWin.getWindowHandle(),
                      nullptr,
                      m_canvasRect.X,
                      m_canvasRect.Y,
                      m_canvasRect.Width, 
                      m_canvasRect.Height,
                      SWP_NOZORDER | SWP_SHOWWINDOW );

        calculateResizingBoxesPos();
        invalidateRect( nullptr );
    }

    void MainWindow::onMouseMove( HWND hWnd, int x, int y, UINT keyFlags )
    {
        updateMousePosLabel( -1, -1 );
        if( m_resizeCanvasFlag )
        {
            m_resizeEndPoint = Point( x, y );
                       
            calculateResizeArea();

            RECT workArea;
            getWorkRect( workArea );
            invalidateRect( &workArea );
        }
    }

    void MainWindow::onLBtnDown( HWND hWnd, bool dblClick, int x, int y, UINT keyFlags )
    {
        for( const auto& [boxType, boxRect] : m_resizingBoxes )
        {
            Point mousePos( x, y );

            if( checkResizeRectCollision( boxRect, mousePos ) )
            {
                SetCapture( m_hWnd );

                m_resizeCanvasFlag = true;
                m_resizeType = boxType;
                
                calculateResizeArea();

                ShowWindow( m_canvasWin.getWindowHandle(), SW_HIDE );

                m_resizeEndPoint = mousePos;

                invalidateRect( nullptr );
            }
        }
    }

    void MainWindow::onLBtnUp( HWND hWnd, int x, int y, UINT keyFlags )
    {
        if( m_resizeCanvasFlag )
        {
            ReleaseCapture();

            m_resizeCanvasFlag = false;
            
            SendMessage( m_canvasWin.getWindowHandle(), 
                         UM_CANVAS_SIZE, 
                         m_xDrawOffset,
                         m_yDrawOffset );

            m_xDrawOffset = 0;
            m_yDrawOffset = 0;

            SendMessage( m_hWnd, UM_CANVAS_SIZE, m_resizeRect.Width, m_resizeRect.Height );
            
            invalidateRect( nullptr );
        }
    }

    void MainWindow::onRBtnDown( HWND hWnd, bool dblClick, int x, int y, UINT keyFlags )
    {
        if( m_resizeCanvasFlag )
        {
            ReleaseCapture();
            m_resizeCanvasFlag = false;
            
            m_xDrawOffset = 0;
            m_yDrawOffset = 0;

            ShowWindow( m_canvasWin.getWindowHandle(), SW_SHOW );

            invalidateRect( nullptr );
        }
    }
    
    void MainWindow::onClose( HWND hWnd )
    {
        if( !m_canvasWin.isSaved() )
        {
            std::wstring mbText{ L"Are you sure you want to close this app?" };
            if( MessageBox( hWnd, mbText.c_str(), APP_NAME.c_str(), MB_YESNO | MB_ICONQUESTION ) == IDYES )
            {
                DestroyWindow( hWnd );
            }
        }
        else
        {
            DestroyWindow( hWnd );
        }
    }

    void MainWindow::onDestroy( HWND hWnd )
    {
        DeleteObject( m_memDC );
        DeleteObject( m_memBitmap );
        DeleteObject( m_toolbarIcons );

        PostQuitMessage( 0 );
    }
    
    bool MainWindow::createToolbar()
    {
        INITCOMMONCONTROLSEX iccx;
        iccx.dwSize = sizeof( INITCOMMONCONTROLSEX );
        iccx.dwICC = ICC_BAR_CLASSES;
        if( !InitCommonControlsEx( &iccx ) )
            return false;

        m_toolbar = CreateWindowEx( NULL,
                                    TOOLBARCLASSNAME,
                                    NULL,
                                    WS_CHILD | WS_VISIBLE | WS_BORDER | TBSTYLE_FLAT | CCS_NODIVIDER,
                                    0, 0, 0, 0,
                                    m_hWnd,
                                    reinterpret_cast<HMENU>( ID_TOOLBAR ),
                                    GetModuleHandle( nullptr ),
                                    NULL );

        if( !m_toolbar )
            return false;

        // If an application uses the CreateWindowEx function to create the 
        // toolbar, the application must send this message to the toolbar before 
        // sending the TB_ADDBITMAP or TB_ADDBUTTONS message. The CreateToolbarEx 
        // function automatically sends TB_BUTTONSTRUCTSIZE, and the size of the 
        // TBBUTTON structure is a parameter of the function.
        SendMessage( m_toolbar, TB_BUTTONSTRUCTSIZE, static_cast<WPARAM>( sizeof( TBBUTTON ) ), 0 );

        constexpr int numButtons = 1 + eTID_THICK_4 - eTID_NEW; // lastIndex - firstIndex
        m_toolbarIcons = loadToolbarIcons( numButtons );
        SendMessage( m_toolbar, TB_SETIMAGELIST, (WPARAM)0, reinterpret_cast<LPARAM>( m_toolbarIcons ) );
        
        TBBUTTON tbButtons[numButtons]{};

#pragma region ButtonInitialization
        
        int imgInx = 0;
        BYTE stateDefault = TBSTATE_ENABLED;
        BYTE styleDefault = BTNS_AUTOSIZE;

        //setting default state & style 
        for( auto& btn : tbButtons )
        {
            btn.fsState = stateDefault;
            btn.fsStyle = styleDefault;
        }

        //setting individual properties
        int btnIndex = 0;
        tbButtons[btnIndex].iBitmap = imgInx++;
        tbButtons[btnIndex].idCommand = eTID_NEW;
        tbButtons[btnIndex].iString = reinterpret_cast<INT_PTR>( L"New" );
        
        ++btnIndex;
        tbButtons[btnIndex].iBitmap = imgInx++;
        tbButtons[btnIndex].idCommand = eTID_LOAD;
        tbButtons[btnIndex].iString = reinterpret_cast<INT_PTR>( L"Load" );
        
        ++btnIndex;
        tbButtons[btnIndex].iBitmap = imgInx++;
        tbButtons[btnIndex].idCommand = eTID_SAVE;
        tbButtons[btnIndex].iString = reinterpret_cast<INT_PTR>( L"Save" );

        ++btnIndex;
        tbButtons[btnIndex].iBitmap = imgInx++;
        tbButtons[btnIndex].idCommand = eTID_SAVE_AS;
        tbButtons[btnIndex].iString = reinterpret_cast<INT_PTR>( L"Save as" );

        //SEPARATOR
        ++btnIndex;
        tbButtons[btnIndex].fsState = NULL;
        tbButtons[btnIndex].fsStyle = TBSTYLE_SEP;

        ++btnIndex;
        tbButtons[btnIndex].iBitmap = imgInx++;
        tbButtons[btnIndex].idCommand = eTID_PENCIL;
        tbButtons[btnIndex].fsState |= TBSTATE_CHECKED;
        tbButtons[btnIndex].fsStyle |= BTNS_CHECKGROUP;
        tbButtons[btnIndex].iString = reinterpret_cast<INT_PTR>( L"Pencil" );

        ++btnIndex;
        tbButtons[btnIndex].iBitmap = imgInx++;
        tbButtons[btnIndex].idCommand = eTID_LINE;
        tbButtons[btnIndex].fsStyle |= BTNS_CHECKGROUP;
        tbButtons[btnIndex].iString = reinterpret_cast<INT_PTR>( L"Line" );

        ++btnIndex;
        tbButtons[btnIndex].iBitmap = imgInx++;
        tbButtons[btnIndex].idCommand = eTID_RECT;
        tbButtons[btnIndex].fsStyle |= BTNS_CHECKGROUP;
        tbButtons[btnIndex].iString = reinterpret_cast<INT_PTR>( L"Rectangle" );

        ++btnIndex;
        tbButtons[btnIndex].iBitmap = imgInx++;
        tbButtons[btnIndex].idCommand = eTID_ELLIPSE;
        tbButtons[btnIndex].fsStyle |= BTNS_CHECKGROUP;
        tbButtons[btnIndex].iString = reinterpret_cast<INT_PTR>( L"Ellipse" );
        
        ++btnIndex;
        tbButtons[btnIndex].iBitmap = imgInx++;
        tbButtons[btnIndex].idCommand = eTID_ERASER;
        tbButtons[btnIndex].fsStyle |= BTNS_CHECKGROUP;
        tbButtons[btnIndex].iString = reinterpret_cast<INT_PTR>( L"Eraser" );
        
        //SEPARATOR
        ++btnIndex;
        tbButtons[btnIndex].fsState = NULL;
        tbButtons[btnIndex].fsStyle = BTNS_SEP;

        ++btnIndex;
        tbButtons[btnIndex].iBitmap = imgInx++;
        tbButtons[btnIndex].idCommand = eTID_MAIN_COLOR;  
        tbButtons[btnIndex].iString = reinterpret_cast<INT_PTR>( L"Color 1" );

        ++btnIndex;
        tbButtons[btnIndex].iBitmap = imgInx++;
        tbButtons[btnIndex].idCommand = eTID_ADDITIONAL_COLOR;
        tbButtons[btnIndex].iString = reinterpret_cast<INT_PTR>( L"Color 2" );
        
        ++btnIndex;
        tbButtons[btnIndex].iBitmap = imgInx++;
        tbButtons[btnIndex].idCommand = eTID_FILL;
        tbButtons[btnIndex].fsStyle |= BTNS_CHECK;
        tbButtons[btnIndex].iString = reinterpret_cast<INT_PTR>( L"Use a fill" );
        
        //SEPARATOR
        ++btnIndex;
        tbButtons[btnIndex].fsState = NULL;
        tbButtons[btnIndex].fsStyle = BTNS_SEP;

        ++btnIndex;
        tbButtons[btnIndex].iBitmap = imgInx++;
        tbButtons[btnIndex].idCommand = eTID_THICK_1;
        tbButtons[btnIndex].fsState |= TBSTATE_CHECKED;
        tbButtons[btnIndex].fsStyle |= BTNS_CHECKGROUP;
        tbButtons[btnIndex].iString = reinterpret_cast<INT_PTR>( L"1" );
        
        ++btnIndex;
        tbButtons[btnIndex].iBitmap = imgInx++;
        tbButtons[btnIndex].idCommand = eTID_THICK_2;
        tbButtons[btnIndex].fsStyle |= BTNS_CHECKGROUP;
        tbButtons[btnIndex].iString = reinterpret_cast<INT_PTR>( L"2" );

        ++btnIndex;
        tbButtons[btnIndex].iBitmap = imgInx++;
        tbButtons[btnIndex].idCommand = eTID_THICK_3;
        tbButtons[btnIndex].fsStyle |= BTNS_CHECKGROUP;
        tbButtons[btnIndex].iString = reinterpret_cast<INT_PTR>( L"3" );

        ++btnIndex;
        tbButtons[btnIndex].iBitmap = imgInx++;
        tbButtons[btnIndex].idCommand = eTID_THICK_4;
        tbButtons[btnIndex].fsStyle |= BTNS_CHECKGROUP;
        tbButtons[btnIndex].iString = reinterpret_cast<INT_PTR>( L"4" );


#pragma endregion ButtonInitialization
            
        SendMessage( m_toolbar, TB_ADDBUTTONS, numButtons, (LPARAM)tbButtons );
        SendMessage( m_toolbar, TB_AUTOSIZE, 0, 0 );
        ShowWindow( m_toolbar, SW_SHOW );

        return true;
    }

    bool MainWindow::createStatusbar()
    {
        m_statusbar = CreateWindowEx( 0L,                    
                                      STATUSCLASSNAME,       
                                      L"",                    
                                      WS_CHILD | WS_VISIBLE,
                                      0, 0, 0, 0,             
                                      m_hWnd,                 
                                      reinterpret_cast<HMENU>( ID_STATUSBAR ),   
                                      GetModuleHandle( nullptr ),                  
                                      nullptr );  
        if( !m_statusbar )
            return false;

        resizeStatusbarParts();

        return true;
    }
    
    HIMAGELIST MainWindow::loadToolbarIcons( const int numIcons )
    {
        auto hInst = GetModuleHandle( nullptr );
        if( !hInst )
            return nullptr;

        int width = 48;
        int height = 48;

        HIMAGELIST toolbarIcons = ImageList_Create( width, height, ILC_COLOR24 | ILC_MASK, numIcons, 0 );
        if( !toolbarIcons )
            return nullptr;
               
        const int imagesIDs[] = { ID_NEW, ID_LOAD, ID_SAVE, ID_SAVE_AS, ID_PENSIL, ID_LINE, 
                                  ID_RECT, ID_ELLIPSE, ID_ERASER };

        HBITMAP hBitmap = nullptr;
        for( const int id : imagesIDs )
        {
            hBitmap = LoadBitmap( hInst, MAKEINTRESOURCE( id ) );
            ImageList_AddMasked( toolbarIcons, hBitmap, 0 );
            DeleteObject( hBitmap );
        }

        //main color button icon
        Bitmap* btnBkgndIcon = nullptr;

        btnBkgndIcon = createColorBitmap( RGB( 0, 0, 0 ), width, height );
        btnBkgndIcon->GetHBITMAP( {}, &hBitmap );
        ImageList_Add( toolbarIcons, hBitmap, 0 );
        DeleteObject( hBitmap );
        delete btnBkgndIcon;

        //additional color button icon
        btnBkgndIcon = createColorBitmap( RGB( 255, 255, 255 ), width, height );
        btnBkgndIcon->GetHBITMAP( {}, &hBitmap );
        ImageList_Add( toolbarIcons, hBitmap, 0 );
        DeleteObject( hBitmap );
        delete btnBkgndIcon;
        
        // add "Use a fill" btn icon
        hBitmap = LoadBitmap( hInst, MAKEINTRESOURCE( ID_FILL ) );
        ImageList_AddMasked( toolbarIcons, hBitmap, 0 );
        DeleteObject( hBitmap );

        if( auto tool = m_canvasWin.getToolkit().getTool( static_cast<EToolbarIDs>( eTID_PENCIL ) ) )
        {
            const auto& array = tool->getPossibleThickness();
            
            //thickness buttons...
            Pen pen( Color::Black );

            const int xOffset = 4;
            const int y = width / 2;
            for( int idx = 0; idx < 4; ++idx )
            {
                btnBkgndIcon = createColorBitmap( RGB( 255, 255, 255 ), width, height );
                Graphics g( btnBkgndIcon );
                pen.SetWidth( static_cast<REAL>( array[idx] ) );
                g.DrawLine( &pen, xOffset, y, width - xOffset, y );
                btnBkgndIcon->GetHBITMAP( {}, &hBitmap );
                ImageList_Add( toolbarIcons, hBitmap, 0 );
                DeleteObject( hBitmap );
                delete btnBkgndIcon;
            }
        }
        return toolbarIcons;
    }

    Bitmap* MainWindow::createColorBitmap( COLORREF color, int width, int height ) const
    {
        Bitmap* bitmap = new Bitmap( width, height );
        Graphics g( bitmap );

        Color newColor;
        newColor.SetFromCOLORREF( color );

        SolidBrush brush( newColor );
        Pen pen( Color::Black );

        pen.SetAlignment( PenAlignmentInset );
        g.FillRectangle( &brush, 0, 0, width, height );
        g.DrawRectangle( &pen, 0, 0, width - 1, height - 1 ); // -1 pen width
        return bitmap;
    }

    bool MainWindow::checkResizeRectCollision( const Gdiplus::Rect& target, const Gdiplus::Point& mousePos ) const noexcept
    {
        return target.Contains( mousePos );
    }

    void MainWindow::drawResizingBoxes( Gdiplus::Graphics* graphics ) const noexcept
    {
        SolidBrush whiteBrush( Color::White );
        Pen pen( Color::Black );
        pen.SetAlignment( PenAlignmentInset );

        for( const auto& [boxType, boxRect] : m_resizingBoxes )
        {
            graphics->FillRectangle( &whiteBrush, boxRect );

            graphics->DrawRectangle( &pen,
                                     boxRect.X,
                                     boxRect.Y,
                                     boxRect.Width - 1, // -1 pen width
                                     boxRect.Height - 1 ); // -1 pen width
        }
    }
    
    void MainWindow::calculateResizingBoxesPos() noexcept
    {
        constexpr int boxOffset = 1;
        const int toolbarHeight = getToolbarHeight();

        m_resizingBoxes[eRR_TOP_LEFT] = Rect( m_canvasRect.X - s_squareSideSize - boxOffset - m_horzScroll.current,
                                              m_canvasRect.Y - s_squareSideSize - boxOffset - m_vertScroll.current,
                                              s_squareSideSize,
                                              s_squareSideSize );

        m_resizingBoxes[eRR_TOP] = Rect( m_canvasRect.X + ( m_canvasRect.Width - s_squareSideSize) / 2 - m_horzScroll.current,
                                         m_canvasRect.Y - s_squareSideSize - boxOffset - m_vertScroll.current,
                                         s_squareSideSize,
                                         s_squareSideSize );

        m_resizingBoxes[eRR_TOP_RIGHT] = Rect( m_canvasRect.X + m_canvasRect.Width + boxOffset - m_horzScroll.current,
                                               m_canvasRect.Y - s_squareSideSize - m_vertScroll.current,
                                               s_squareSideSize,
                                               s_squareSideSize );

        m_resizingBoxes[eRR_LEFT] = Rect( m_canvasRect.X - s_squareSideSize - boxOffset - m_horzScroll.current,
                                          m_canvasRect.Y + ( m_canvasRect.Height - s_squareSideSize ) / 2 - m_vertScroll.current,
                                          s_squareSideSize,
                                          s_squareSideSize );

        m_resizingBoxes[eRR_RIGHT] = Rect( m_canvasRect.X + m_canvasRect.Width + boxOffset - m_horzScroll.current,
                                           m_canvasRect.Y + (m_canvasRect.Height - s_squareSideSize) / 2 - m_vertScroll.current,
                                           s_squareSideSize,
                                           s_squareSideSize );

        m_resizingBoxes[eRR_BOTTOM_LEFT] = Rect( m_canvasRect.X - s_squareSideSize - boxOffset - m_horzScroll.current,
                                                 m_canvasRect.Y + m_canvasRect.Height + boxOffset - m_vertScroll.current,
                                                 s_squareSideSize, 
                                                 s_squareSideSize );

        m_resizingBoxes[eRR_BOTTOM] = Rect( m_canvasRect.X + ( m_canvasRect.Width - s_squareSideSize) / 2 - m_horzScroll.current,
                                            m_canvasRect.Y + m_canvasRect.Height + boxOffset - m_vertScroll.current,
                                            s_squareSideSize,
                                            s_squareSideSize );

        m_resizingBoxes[eRR_BOTTOM_RIGHT] = Rect( m_canvasRect.X + m_canvasRect.Width + boxOffset - m_horzScroll.current,
                                                  m_canvasRect.Y + m_canvasRect.Height + boxOffset - m_vertScroll.current,
                                                  s_squareSideSize,
                                                  s_squareSideSize );
                
    }

    void MainWindow::calculateÑanvasPos( const RECT& rect ) noexcept
    {
        const int x = ( std::max )( s_canvasWinOffset.left, 
                                   ( rect.right - m_canvasRect.Width ) / 2 - s_horzScrollbarThickness + s_canvasWinOffset.left );

        const int y = ( std::max )( rect.top + s_canvasWinOffset.top,
                                    rect.top + ( ( rect.bottom - rect.top - m_canvasRect.Height ) / 2 - s_vertScrollbarThickness ) + 
                                    s_canvasWinOffset.top );

        m_canvasRect = Rect( x, y, m_canvasRect.Width, m_canvasRect.Height );
    }

    void MainWindow::calculateResizeArea() noexcept
    {
        switch( m_resizeType )
        {
            case eRR_TOP_LEFT:
            {
                m_resizeRect.X = m_resizeEndPoint.X;
                m_resizeRect.Y = m_resizeEndPoint.Y;
                m_resizeRect.Width = m_canvasRect.X - m_resizeEndPoint.X + m_canvasRect.Width;
                m_resizeRect.Height = m_canvasRect.Y - m_resizeEndPoint.Y + m_canvasRect.Height;

                m_xDrawOffset = m_canvasRect.X - m_resizeEndPoint.X;
                m_yDrawOffset = m_canvasRect.Y - m_resizeEndPoint.Y;
                break;
            }
            case eRR_TOP_RIGHT:
            {
                m_resizeRect.X = m_canvasRect.X;
                m_resizeRect.Y = m_resizeEndPoint.Y;
                m_resizeRect.Width = m_resizeEndPoint.X - m_canvasRect.X;
                m_resizeRect.Height = m_canvasRect.Y - m_resizeEndPoint.Y + m_canvasRect.Height;
                
                m_yDrawOffset = m_canvasRect.Y - m_resizeEndPoint.Y;
                break;
            }
            case eRR_TOP:
            {
                m_resizeRect.X = m_canvasRect.X;
                m_resizeRect.Y = m_resizeEndPoint.Y;
                m_resizeRect.Width = m_canvasRect.Width;
                m_resizeRect.Height = m_canvasRect.Y - m_resizeEndPoint.Y + m_canvasRect.Height;

                m_yDrawOffset = m_canvasRect.Y - m_resizeEndPoint.Y;
                break;
            }
            case eRR_BOTTOM_LEFT:
            {
                m_resizeRect.X = m_resizeEndPoint.X;
                m_resizeRect.Y = m_canvasRect.Y;
                m_resizeRect.Width = m_canvasRect.X - m_resizeEndPoint.X + m_canvasRect.Width;
                m_resizeRect.Height = m_resizeEndPoint.Y - m_canvasRect.Y;

                m_xDrawOffset = m_canvasRect.X - m_resizeEndPoint.X;
                break;
            }
            case eRR_BOTTOM_RIGHT:
            {
                m_resizeRect.X = m_canvasRect.X;
                m_resizeRect.Y = m_canvasRect.Y;
                m_resizeRect.Width = m_resizeEndPoint.X - m_canvasRect.X;
                m_resizeRect.Height = m_resizeEndPoint.Y - m_canvasRect.Y;
                break;
            }
            case eRR_BOTTOM:
            {
                m_resizeRect.X = m_canvasRect.X;
                m_resizeRect.Y = m_canvasRect.Y;
                m_resizeRect.Width = m_canvasRect.Width;
                m_resizeRect.Height = m_resizeEndPoint.Y - m_canvasRect.Y;
                break;
            }
            case eRR_RIGHT:
            {
                m_resizeRect.X = m_canvasRect.X;
                m_resizeRect.Y = m_canvasRect.Y;
                m_resizeRect.Width = m_resizeEndPoint.X - m_canvasRect.X;
                m_resizeRect.Height = m_canvasRect.Height;
                break;
            }
            case eRR_LEFT:
            {
                m_resizeRect.X = m_resizeEndPoint.X;
                m_resizeRect.Y = m_canvasRect.Y;
                m_resizeRect.Width = m_canvasRect.X - m_resizeEndPoint.X + m_canvasRect.Width;
                m_resizeRect.Height = m_canvasRect.Height;

                m_xDrawOffset = m_canvasRect.X - m_resizeEndPoint.X;
                break;
            }
        }
        m_resizeRect.Width = ( std::max )( 1, m_resizeRect.Width );
        m_resizeRect.Height = ( std::max )( 1, m_resizeRect.Height );
    }

    void MainWindow::drawResizeArea( Gdiplus::Graphics* graphics ) const noexcept
    {
        Pen pen( Color::Black, 2);
        pen.SetDashStyle( DashStyleDash );
        
        graphics->DrawRectangle( &pen, m_resizeRect.X, m_resizeRect.Y, m_resizeRect.Width, m_resizeRect.Height );
    }

    int MainWindow::getToolbarHeight() const noexcept
    {
        RECT toolbarRc;
        if( GetWindowRect( m_toolbar, &toolbarRc ) )
            return toolbarRc.bottom - toolbarRc.top;
        return 0;
    }

    inline int MainWindow::getStatusbarHeight() const noexcept
    {
        RECT statusbarRc;
        if( GetWindowRect( m_statusbar, &statusbarRc ) )
            return statusbarRc.bottom - statusbarRc.top;
        return 0;
    }

    void MainWindow::getWorkRect( RECT& outWorkRect ) const noexcept
    {
        if( !GetClientRect( m_hWnd, &outWorkRect ) )
            return;

        outWorkRect.top = getToolbarHeight();
        outWorkRect.bottom -= getStatusbarHeight();
    }

    void MainWindow::resizeScrollbars( int winWidth, int winHeight ) noexcept
    {
        SCROLLINFO si{ sizeof( si ) };

        m_horzScroll.max = ( std::max )( m_canvasRect.Width - winWidth + 
                                       ( s_canvasWinOffset.left + s_canvasWinOffset.right ), 0L );

        m_horzScroll.current = ( std::min )( m_horzScroll.current, m_horzScroll.max );
        si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
        si.nMin = m_horzScroll.min;
        si.nMax = m_canvasRect.Width + ( s_canvasWinOffset.left + s_canvasWinOffset.right );
        si.nPage = winWidth;
        si.nPos = m_horzScroll.current;
        SetScrollInfo( m_hWnd, SB_HORZ, &si, true );


        m_vertScroll.max = ( std::max )( 0L, 
                                         m_canvasRect.Height -
                                         winHeight + 
                                       ( getToolbarHeight() + 
                                         getStatusbarHeight() +
                                         s_canvasWinOffset.top + 
                                         s_canvasWinOffset.bottom ) );

        m_vertScroll.current = ( std::min )( m_vertScroll.current, m_vertScroll.max );
        si.fMask = SIF_RANGE | SIF_PAGE | SIF_POS;
        si.nMin = m_vertScroll.min;
        si.nMax = m_canvasRect.Height +
                  ( getToolbarHeight() +
                  getStatusbarHeight() +
                  s_canvasWinOffset.top + 
                  s_canvasWinOffset.bottom );
        si.nPage = winHeight;
        si.nPos = m_vertScroll.current;
        SetScrollInfo( m_hWnd, SB_VERT, &si, true );
    }

    void MainWindow::resizeStatusbarParts() noexcept
    {
        int partWidth[3];
        partWidth[0] = 160;
        partWidth[1] = partWidth[0] + 170;
        partWidth[2] = partWidth[1] + 80;
        SendMessage( m_statusbar, SB_SETPARTS, 3, reinterpret_cast<LPARAM>( partWidth ) );
    }

    void MainWindow::updateMousePosLabel( const int x, const int y ) noexcept
    {
        std::wstring mousePosLabel = L"Mouse pos: " +
                                       std::to_wstring( x ) +
                                       L", " + std::to_wstring( y );
        SendMessage( m_statusbar, SB_SETTEXT, 0, reinterpret_cast<LPARAM>( mousePosLabel.c_str() ) );
    }

    void MainWindow::updateCanvasSizeLabel() noexcept
    {
        std::wstring canvasSizeLabel = L"Canvas size: " +
                                       std::to_wstring( m_canvasRect.Width ) +
                                       L" x " + std::to_wstring( m_canvasRect.Height );
        SendMessage( m_statusbar, SB_SETTEXT, 1, reinterpret_cast<LPARAM>( canvasSizeLabel.c_str() ) );    
    }

    void MainWindow::updateScaleLabel( const int scale ) noexcept
    {
        std::wstring scaleLabel = L"Scale: " + std::to_wstring( scale ) + L"%";
        SendMessage( m_statusbar, SB_SETTEXT, 2, reinterpret_cast<LPARAM>( scaleLabel.c_str() ) );
    }

} //namespace PaintLite