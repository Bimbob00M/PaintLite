#pragma comment(lib, "gdiplus.lib")
#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "Comctl32.lib")

#include "global.h"
#include "framework.h"
#include "gdiplus_starup_wrapper.h"
#include "main_window.h"

int APIENTRY wWinMain( _In_ HINSTANCE hInstance,
                       _In_opt_ HINSTANCE /*hPrevInstance*/,
                       _In_ LPWSTR    /*lpCmdLine*/,
                       _In_ int       nCmdShow )
{
    // Gdiplus should be deinitialize after class objects that use Gdiplus functions.
    UINT_PTR gdiplusToken{};
    PaintLite::GdiplusStarupWrapper gdiplus( gdiplusToken );

    int screenWidth = GetSystemMetrics( SM_CXVIRTUALSCREEN );
    int screenHeight = GetSystemMetrics( SM_CYVIRTUALSCREEN );
    int winWidth = screenWidth / 2;
    int winHeight = screenHeight / 2;

    PaintLite::MainWindow win;
    if( !win.create( PaintLite::APP_NAME.c_str(),
                     WS_OVERLAPPEDWINDOW,
                     NULL,
                     ( screenWidth - winWidth ) / 2,
                     ( screenHeight - winHeight ) / 2,
                     winWidth,
                     winHeight ) )
    {
        return 0;
    }
    win.show( nCmdShow );
    win.update();
    
    MSG msg;
    while( GetMessage( &msg, nullptr, 0, 0 ) )
    {
        TranslateMessage( &msg );
        DispatchMessage( &msg );
    }

    return msg.wParam;
}
