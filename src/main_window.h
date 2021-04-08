#pragma once

#include <Windows.h>
#include <commctrl.h>

#include "base_window.hpp"
#include "canvas_window.h"
#include "drawing_tool.h"

namespace PaintLite
{
    class MainWindow : public BaseWindow<MainWindow>
    {
    public:        
        inline PCWSTR  className() const override { return L"Main Window"; };
        LRESULT handleMessage( UINT msg, WPARAM wParam, LPARAM lParam ) override;

    protected:
        virtual void initWndClass( WNDCLASS& outWndClass ) const override;
        
        virtual bool onCreate( HWND hWnd, LPCREATESTRUCT lpCreateStruct );
        virtual void onCommand( HWND hWnd, int id, HWND hwndCtl, UINT codeNotify );
        virtual void onPaint( HWND hWnd );
        virtual void onSize( HWND hWnd, UINT state, int width, int height );
        virtual void onMouseMove( HWND hWnd, int x, int y, UINT keyFlags );
        virtual void onLBtnDown( HWND hWnd, bool dblClick, int x, int y, UINT keyFlags );
        virtual void onLBtnUp( HWND hWnd, int x, int y, UINT keyFlags );
        virtual void onRBtnDown( HWND hWnd, bool dblClick, int x, int y, UINT keyFlags );
        virtual void onClose( HWND hWnd );
        virtual void onDestroy( HWND hWnd );

        bool createToolbar();

    private:        
        enum EResizeRect { eRR_NONE, eRR_BOTTOM, eRR_RIGHT, eRR_BOTTOM_RIGHT };

        CanvasWindow m_canvasWin;
        HIMAGELIST m_toolbarIcons{ nullptr };
        HWND m_toolbar{ nullptr };
        
        Gdiplus::Rect m_canvasRect{};

        Gdiplus::Rect m_bottomCanvasWin{};
        Gdiplus::Rect m_rightCanvasWin{};
        Gdiplus::Rect m_bottomRightCanvasWin{};

        Gdiplus::Point m_resizeEndPoint{};

        EResizeRect m_resizeType{};
        bool m_resizeCanvasWinFlag{ false };

        HIMAGELIST loadToolbarIcons( const int numButtons );
        Gdiplus::Bitmap* createColorBitmap( COLORREF color, int width, int height ) const;

        bool checkResizeRectCollision( const Gdiplus::Rect& target, const Gdiplus::Point& mousePos ) const noexcept;
        void drawResizeRects( Gdiplus::Graphics* graphics ) const noexcept;
        void drawResizeArea( Gdiplus::Graphics* graphics ) const noexcept;

        const int getToolbarHeight() const noexcept;

        void calculateCanvasWinRect( RECT& clientArea, const int toolbarHeight ) noexcept;
        void calculateResizeRectsPos() noexcept;
    };

}//namespace Paint