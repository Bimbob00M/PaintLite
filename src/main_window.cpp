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
    
    LRESULT MainWindow::handleMessage( UINT msg, WPARAM wParam, LPARAM lParam )
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
            HANDLE_MSG( m_hWnd, WM_CLOSE, onClose );
            HANDLE_MSG( m_hWnd, WM_DESTROY, onDestroy );

            case UM_CANVAS_SIZE:
                onCanvasSize( wParam, lParam );
                break;

            default:
                return DefWindowProc( m_hWnd, msg, wParam, lParam );
        }
        return 0;
    }

    void MainWindow::initWndClass( WNDCLASS& outWndClass ) const
    {
        BaseWindow::initWndClass( outWndClass );

        outWndClass.hbrBackground = GetStockBrush( LTGRAY_BRUSH );
        outWndClass.hIcon = LoadIcon( GetModuleHandle( nullptr ), MAKEINTRESOURCE( IDI_PAINTLITE ) );
        outWndClass.hCursor = LoadCursor( nullptr, IDC_ARROW );
    }

    bool MainWindow::onCreate( HWND hWnd, LPCREATESTRUCT lpCreateStruct )
    {
        if( !createToolbar() )
            return false;
        
        RECT clientRc; 
        GetClientRect( m_hWnd, &clientRc );
        const int toolbarHeight = getToolbarHeight();

        const int topLeftOffset = 5;
        const int bottomRightOffset = 15; // topLeftOffset * 2 + 5( reserve for rectangles resizing)

        m_canvasRect = Rect( clientRc.left + topLeftOffset,
                             clientRc.top + topLeftOffset + toolbarHeight,
                             clientRc.right - bottomRightOffset,
                             clientRc.bottom - bottomRightOffset - toolbarHeight );

        auto result = m_canvasWin.create( nullptr,
                                          WS_CHILD | WS_VISIBLE,
                                          0,
                                          m_canvasRect.X,
                                          m_canvasRect.Y,
                                          m_canvasRect.Width,
                                          m_canvasRect.Height,
                                          m_hWnd,
                                          reinterpret_cast<HMENU>( ID_CANVAS_WIN ) );

        calculateResizeRectsPos();        

        return result;
    }

    void MainWindow::onCommand( HWND hWnd, int id, HWND hwndCtl, UINT codeNotify )
    {
        switch( id )
        {
            case eTID_NEW:
            case eTID_LOAD:
            case eTID_SAVE:
            case eTID_SAVE_AS:
            case eTID_PENCIL:
            case eTID_LINE:
            case eTID_RECT:
            case eTID_ELLIPSE:
            case eTID_ERASER:
            case eTID_THICK_1:
            case eTID_THICK_2:
            case eTID_THICK_3:
            case eTID_THICK_4:
            {
                SendMessage( m_canvasWin.getWindowHandle(), UM_TOOLBAR_COMMAND, id, 0 );
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

                    SendMessage( m_toolbar, TB_CHANGEBITMAP, id, colorBtnIndex - numSeparators );
                    SendMessage( m_canvasWin.getWindowHandle(), UM_TOOLBAR_COMMAND, id, static_cast<LPARAM>( s_rgbCurrent ) );
                }
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
            
        HBRUSH bkgnd = GetStockBrush( LTGRAY_BRUSH );
        if( m_resizeCanvasWinFlag )
        {
            const int width = ps.rcPaint.right - ps.rcPaint.left;
            const int height = ps.rcPaint.bottom - ps.rcPaint.top;

            auto dc = m_memDC ? m_memDC : ps.hdc;
            Graphics g( dc ); 
            FillRect( dc, &ps.rcPaint, bkgnd );
            drawResizeRects( &g );

            if( m_memDC )
                BitBlt( ps.hdc, 0, 0, width, height, m_memDC, 0, 0, SRCCOPY );
        }
        else
        {
            Graphics g( ps.hdc );
            FillRect( ps.hdc, &ps.rcPaint, bkgnd );
            drawResizeRects( &g );
        }
        DeleteObject( bkgnd );

        EndPaint( hWnd, &ps );
    }

    void MainWindow::onSize( HWND hWnd, UINT state, int width, int height )
    {  
        SendMessage( m_toolbar, TB_AUTOSIZE, state, MAKELPARAM( width, height ) );        
    }

    void MainWindow::onCanvasSize( int width, int height )
    {
        m_canvasRect.Width = width;
        m_canvasRect.Height = height;
        calculateResizeRectsPos();
        invalidateRect( nullptr );
    }

    void MainWindow::onMouseMove( HWND hWnd, int x, int y, UINT keyFlags )
    {

        if( m_resizeCanvasWinFlag )
        {
            RECT clientArea;
            GetClientRect( m_hWnd, &clientArea );
                       
            bool mouseCompletelyOutsideClientArea = ( x < clientArea.left && y < clientArea.top ) ||
                                                    ( x > clientArea.right && y > clientArea.bottom );

            if( mouseCompletelyOutsideClientArea )
                return;

            m_resizeEndPoint = Point( x, y );

            int width = m_resizeEndPoint.X - m_canvasRect.X;
            int height = m_resizeEndPoint.Y - m_canvasRect.Y;

            if( m_resizeType == eRR_BOTTOM )
            {
                width = m_canvasRect.Width;
            }
            else if( m_resizeType == eRR_RIGHT )
            {
                height = m_canvasRect.Height;
            }

            const int offset = 15;
            m_canvasRect.Width = std::clamp( width, 1, static_cast<int>( clientArea.right - offset ) );
            m_canvasRect.Height = std::clamp( height, 1, static_cast<int>( clientArea.bottom - offset - getToolbarHeight() ) );

            SetWindowPos( m_canvasWin.getWindowHandle(), 0, 0, 0, m_canvasRect.Width, m_canvasRect.Height, SWP_NOMOVE );
            calculateResizeRectsPos();

            invalidateRect( nullptr );
        }
    }

    void MainWindow::onLBtnDown( HWND hWnd, bool dblClick, int x, int y, UINT keyFlags )
    {
        SetCapture( m_hWnd );
        Point mousePos( x, y );

        if( checkResizeRectCollision( m_bottomCanvasWin, mousePos ) )
        {
            m_resizeCanvasWinFlag = true;
            m_resizeType = eRR_BOTTOM;
        }
        else if( checkResizeRectCollision( m_rightCanvasWin, mousePos ) )
        {
            m_resizeCanvasWinFlag = true;
            m_resizeType = eRR_RIGHT;
        }
        else if( checkResizeRectCollision( m_bottomRightCanvasWin, mousePos ) )
        {
            m_resizeCanvasWinFlag = true;
            m_resizeType = eRR_BOTTOM_RIGHT;
        }

        if( m_resizeCanvasWinFlag )
        {
            m_resizeEndPoint = Point( x, y );

            if( HDC hWinDC = GetDC( m_hWnd ) )
            {
                if( m_memDC = CreateCompatibleDC( hWinDC ) )
                {
                    RECT clientRect;
                    GetClientRect( m_hWnd, &clientRect );
                    if( m_memBitmap = CreateCompatibleBitmap( hWinDC, clientRect.right, clientRect.bottom ) )
                    {
                        DeleteObject( SelectObject( m_memDC, m_memBitmap ) );
                        ReleaseDC( m_hWnd, hWinDC );
                    }
                    ReleaseDC( m_hWnd, hWinDC );
                }
            }

            auto styles = GetWindowLong( m_hWnd, GWL_STYLE );
            SetWindowLong( m_hWnd, GWL_STYLE, styles | WS_CLIPCHILDREN );

            invalidateRect( nullptr );
        }
    }

    void MainWindow::onLBtnUp( HWND hWnd, int x, int y, UINT keyFlags )
    {
        ReleaseCapture();

        if( m_resizeCanvasWinFlag )
        {
            m_resizeCanvasWinFlag = false;
            m_resizeType = eRR_NONE;

            DeleteObject( m_memDC );
            DeleteObject( m_memBitmap );

            auto styles = GetWindowLong( m_hWnd, GWL_STYLE );
            SetWindowLong( m_hWnd, GWL_STYLE, styles ^ WS_CLIPCHILDREN );

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
        
        //SEPARATOR
        ++btnIndex;
        tbButtons[btnIndex].fsState = NULL;
        tbButtons[btnIndex].fsStyle = BTNS_SEP;

        ++btnIndex;
        tbButtons[btnIndex].iBitmap = imgInx++;
        tbButtons[btnIndex].idCommand = eTID_THICK_1;
        tbButtons[btnIndex].iString = reinterpret_cast<INT_PTR>( L"1" );
        
        ++btnIndex;
        tbButtons[btnIndex].iBitmap = imgInx++;
        tbButtons[btnIndex].idCommand = eTID_THICK_2;
        tbButtons[btnIndex].iString = reinterpret_cast<INT_PTR>( L"2" );

        ++btnIndex;
        tbButtons[btnIndex].iBitmap = imgInx++;
        tbButtons[btnIndex].idCommand = eTID_THICK_3;
        tbButtons[btnIndex].iString = reinterpret_cast<INT_PTR>( L"3" );

        ++btnIndex;
        tbButtons[btnIndex].iBitmap = imgInx++;
        tbButtons[btnIndex].idCommand = eTID_THICK_4;
        tbButtons[btnIndex].iString = reinterpret_cast<INT_PTR>( L"4" );


#pragma endregion ButtonInitialization
            
        SendMessage( m_toolbar, TB_ADDBUTTONS, numButtons, (LPARAM)tbButtons );
        SendMessage( m_toolbar, TB_AUTOSIZE, 0, 0 );
        ShowWindow( m_toolbar, SW_SHOW );

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

        //thickness buttons...
        Pen pen( Color::Black );
        DrawingTool* tool = nullptr;

        auto it = m_canvasWin.getToolkit().find( static_cast<EToolbarIDs>( eTID_PENCIL ) );
        if( it != m_canvasWin.getToolkit().end() )        
            tool = it->second.get();        
        auto array = tool->getPossibleThickness();

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

    void MainWindow::drawResizeRects( Gdiplus::Graphics* graphics ) const noexcept
    {
        SolidBrush whiteBrush( Color::White );
        Pen pen( Color::Black );
        pen.SetAlignment( PenAlignmentInset );

        graphics->FillRectangle( &whiteBrush, m_bottomCanvasWin );
        graphics->FillRectangle( &whiteBrush, m_bottomRightCanvasWin );
        graphics->FillRectangle( &whiteBrush, m_rightCanvasWin );

        graphics->DrawRectangle( &pen,
                                 m_bottomCanvasWin.X, 
                                 m_bottomCanvasWin.Y,
                                 m_bottomCanvasWin.Width - 1, 
                                 m_bottomCanvasWin.Height - 1 ); // -1 pen width

        graphics->DrawRectangle( &pen, 
                                 m_bottomRightCanvasWin.X, 
                                 m_bottomRightCanvasWin.Y,
                                 m_bottomRightCanvasWin.Width - 1, 
                                 m_bottomRightCanvasWin.Height - 1 ); // -1 pen width

        graphics->DrawRectangle( &pen, 
                                 m_rightCanvasWin.X, 
                                 m_rightCanvasWin.Y,
                                 m_rightCanvasWin.Width - 1, 
                                 m_rightCanvasWin.Height - 1 ); // -1 pen width
    }
    
    void MainWindow::calculateResizeRectsPos() noexcept
    {
        const int toolbarHeight = getToolbarHeight();

        const int sizeRectWidth = 6;
        const int sizeRectOffset = 1;

        m_bottomCanvasWin = Rect{ ( m_canvasRect.Width - sizeRectWidth ) / 2,
                                    m_canvasRect.Height + sizeRectOffset + sizeRectWidth + toolbarHeight,
                                    sizeRectWidth,
                                    sizeRectWidth };

        m_rightCanvasWin = Rect{ m_canvasRect.Width + sizeRectOffset + sizeRectWidth,
                                    ( m_canvasRect.Height - sizeRectWidth ) / 2 + toolbarHeight,
                                    sizeRectWidth,
                                    sizeRectWidth };

        m_bottomRightCanvasWin = Rect{ m_canvasRect.Width + sizeRectOffset + sizeRectWidth,
                                        m_canvasRect.Height + sizeRectOffset + sizeRectWidth + toolbarHeight,
                                        sizeRectWidth,
                                        sizeRectWidth };
        
    }

    int MainWindow::getToolbarHeight() const noexcept
    {
        RECT rcToolbarWin;
        GetWindowRect( m_toolbar, &rcToolbarWin );
        return rcToolbarWin.bottom - rcToolbarWin.top;
    }

} //namespace PaintLite