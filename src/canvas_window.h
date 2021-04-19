#pragma once

#include <memory>
#include <unordered_map>

#include "base_window.hpp"
#include "canvas.h"
#include "drawer.h"
#include "drawing_toolkit.h"

namespace PaintLite
{
    class CanvasWindow : public BaseWindow<CanvasWindow>
    {
    public:
        CanvasWindow();

        inline PCWSTR  className() const override { return L"DrawingArea"; }
        LRESULT handleMessage( UINT msg, WPARAM wParam, LPARAM lParam ) override;
        
        inline const Canvas& getCanvas() const noexcept { return m_canvas; }
        inline Canvas* getCanvas() noexcept { return &m_canvas; }
        
        inline const DrawingToolkit& getToolkit() const noexcept { return m_toolkit; }
        inline DrawingToolkit& getToolkit() noexcept { return m_toolkit; }

        LRESULT clear() noexcept;

        inline bool isSaved() const noexcept { return m_saveState; }

    protected:
        virtual void initWndClass( WNDCLASSEX& outWndClass ) const override;

        virtual bool onCreate( HWND hWnd, LPCREATESTRUCT lpCreateStruct );
        virtual void onCommand( HWND hWnd, int id, HWND hwndCtl, UINT codeNotify );
        virtual void onToolbarCommand( int id, UINT codeNotify );
        virtual void onPaint( HWND hWnd );
        virtual void onSize( HWND hWnd, UINT state, int width, int height );
        virtual void onMouseMove( HWND hWnd, int x, int y, UINT keyFlags );
        virtual void onMouseWheel( HWND hWnd, int x, int y, int delta, UINT keyFlags );
        virtual void onLBtnDown( HWND hWnd, bool dblClick, int x, int y, UINT keyFlags );
        virtual void onLBtnUp( HWND hWnd, int x, int y, UINT keyFlags );
        virtual void onRBtnDown( HWND hWnd, bool dblClick, int x, int y, UINT keyFlags );
        virtual void onDestroy( HWND hWnd);

    private:
        Canvas m_canvas{};
        Canvas m_drawerCanvas{};
        Drawer m_drawer{};
        DrawingToolkit m_toolkit{};

        float m_scalingFactor{ 1.F };

        HDC m_memDC{ nullptr };
        HBITMAP m_memBitmap{ nullptr };

        int m_xDrawOffset{};
        int m_yDrawOffset{};

        bool m_saveState{ false };
        bool m_resizedWithOffsetRedraw{ false };

        inline void resetScale() noexcept;
    };
}