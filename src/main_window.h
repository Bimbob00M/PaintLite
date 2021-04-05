#pragma once

#include <Windows.h>

#include "src/base_window.hpp"
#include "src/canvas.h"
#include "src/drawer.h"
#include "src/drawing_toolkit.h"
#include "src/pencil.h"
#include "src/line.h"
#include "src/rectangle.h"
#include "src/ellipse.h"

namespace PaintLite
{    
    class MainWindow : public BaseWindow<MainWindow>
    {
    public:
        MainWindow();

        PCWSTR  className() const;
        LRESULT handleMessage( UINT msg, WPARAM wParam, LPARAM lParam );

    protected:
        virtual void initWndClass( WNDCLASS& outWndClass ) const;

        virtual LRESULT onPaint( WPARAM wParam, LPARAM lParam );
        virtual LRESULT onSize( WPARAM wParam, LPARAM lParam );
        virtual LRESULT onClose( WPARAM wParam, LPARAM lParam );

    private:
        //DrawingToolkit m_toolkit{};
        Canvas m_winCanvas;
        Canvas m_drawerCanvas;
        Drawer m_drawer;

        Pencil m_tool;   

        HWND m_hToolbar;

        bool createToolbar( HWND hParent );


    };

    //**********************************************************************************************************

    inline PCWSTR MainWindow::className() const
    {
        return L"Main Window";
    }

    //--------------------------------------------------------------------------------------------------------------

    inline void MainWindow::initWndClass( WNDCLASS& outWndClass ) const
    {
        BaseWindow::initWndClass( outWndClass );

        outWndClass.hIcon = LoadIcon( GetModuleHandle( nullptr ), MAKEINTRESOURCE( IDI_PAINTLITE ) );
        outWndClass.hCursor = LoadCursor( nullptr, IDC_ARROW );
    }

}//namespace Paint