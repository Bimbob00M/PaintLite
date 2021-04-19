#pragma once

#include <Windows.h>
#include <commctrl.h>

#include <unordered_map>

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
        virtual void initWndClass( WNDCLASSEX& outWndClass ) const override;
        
        virtual bool onCreate( HWND hWnd, LPCREATESTRUCT lpCreateStruct );
        virtual void onCommand( HWND hWnd, int id, HWND hwndCtl, UINT codeNotify );
        virtual void onPaint( HWND hWnd );
        virtual void onSize( HWND hWnd, UINT state, int width, int height );
        virtual void onHScroll( HWND hWnd, HWND hwndCtl, UINT code, int pos );
        virtual void onVScroll( HWND hWnd, HWND hwndCtl, UINT code, int pos );
        virtual void onCanvasSize( int width, int height );
        virtual void onMouseMove( HWND hWnd, int x, int y, UINT keyFlags );
        virtual void onLBtnDown( HWND hWnd, bool dblClick, int x, int y, UINT keyFlags );
        virtual void onLBtnUp( HWND hWnd, int x, int y, UINT keyFlags );
        virtual void onRBtnDown( HWND hWnd, bool dblClick, int x, int y, UINT keyFlags );
        virtual void onClose( HWND hWnd );
        virtual void onDestroy( HWND hWnd );
        
    private:        
        enum EResizingBox { eRR_TOP_LEFT, eRR_TOP_RIGHT, eRR_TOP, eRR_BOTTOM_LEFT, eRR_BOTTOM_RIGHT, eRR_BOTTOM, eRR_RIGHT, eRR_LEFT };

        struct ScrollData
        {
            int max;
            int min;
            int current;
        };

        CanvasWindow m_canvasWin;
        HIMAGELIST m_toolbarIcons{ nullptr };

        std::unordered_map<EResizingBox, Gdiplus::Rect> m_resizingBoxes;
        Gdiplus::Rect m_canvasRect{};
        Gdiplus::Rect m_resizeRect{};

        ScrollData m_horzScroll;
        ScrollData m_vertScroll;

        Gdiplus::Point m_resizeEndPoint{};
        
        HWND m_toolbar{ nullptr };
        HWND m_statusbar{ nullptr };

        HDC m_memDC{ nullptr };
        HBITMAP m_memBitmap{ nullptr };
        
        int m_xDrawOffset{};
        int m_yDrawOffset{};

        EResizingBox m_resizeType{};
        bool m_resizeCanvasFlag{ false };
        
        bool createToolbar();
        bool createStatusbar();

        HIMAGELIST loadToolbarIcons( const int numButtons );
        Gdiplus::Bitmap* createColorBitmap( COLORREF color, int width, int height ) const;

        bool checkResizeRectCollision( const Gdiplus::Rect& target, const Gdiplus::Point& mousePos ) const noexcept;
        void drawResizingBoxes( Gdiplus::Graphics* graphics ) const noexcept;
        void calculateResizingBoxesPos() noexcept;

        // canvasWidth and canvasHeight must reserve place for canvas resizing boxes
        void calculate—anvasPos( const RECT& rect ) noexcept;

        void calculateResizeArea() noexcept;
        void drawResizeArea( Gdiplus::Graphics* graphics ) const noexcept;

        inline int getToolbarHeight() const noexcept;    
        inline int getStatusbarHeight() const noexcept;

        // Client area exclude toolbar height
        inline void getWorkRect(RECT& outWorkRect) const noexcept;

        void resizeScrollbars( int winWidth, int winHeight ) noexcept;
        void resizeStatusbarParts() noexcept;

        void updateMousePosLabel( const int x, const int y ) noexcept;
        void updateCanvasSizeLabel() noexcept;
        void updateScaleLabel( const int scale ) noexcept;
    };

}//namespace Paint