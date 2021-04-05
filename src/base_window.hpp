#pragma once

#include <Windows.h>

#include "res/resource.h"

namespace PaintLite
{
    template <class DERIVED_TYPE>
    class BaseWindow
    {
    public:
        BaseWindow() : m_hWnd( nullptr )
        {}

        virtual ~BaseWindow()
        {
            SendMessage( m_hWnd, WM_CLOSE, NULL, NULL );
        }

        static LRESULT CALLBACK windowProc( HWND hwnd, 
                                            UINT uMsg, 
                                            WPARAM wParam,
                                            LPARAM lParam );

        BOOL create( PCWSTR lpWindowName, 
                     DWORD dwStyle, 
                     DWORD dwExStyle = 0,
                     int x = CW_USEDEFAULT, 
                     int y = CW_USEDEFAULT,
                     int nWidth = CW_USEDEFAULT, 
                     int nHeight = CW_USEDEFAULT,
                     HWND hWndParent = 0, 
                     HMENU hMenu = 0 );

        BOOL show( int nCmdShow );
        BOOL update();
        
        HWND getWindowHandle() const;

    protected:
        HWND m_hWnd;

        virtual PCWSTR className() const = 0;
        virtual LRESULT handleMessage( UINT uMsg, WPARAM wParam, LPARAM lParam ) = 0;

        virtual void initWndClass( WNDCLASS& outWndClass ) const;

    private:
        BaseWindow( const BaseWindow<DERIVED_TYPE>& ) = delete;
        BaseWindow<DERIVED_TYPE>& operator=( const BaseWindow<DERIVED_TYPE>& ) = delete;

        ATOM registerClass() const;
    };

    //**********************************************************************************************************

    template <class DERIVED_TYPE>
    LRESULT BaseWindow<DERIVED_TYPE>::windowProc( HWND hwnd, 
                                                  UINT uMsg,
                                                  WPARAM wParam, 
                                                  LPARAM lParam )
    {
        DERIVED_TYPE* pThis = nullptr;

        if( uMsg == WM_NCCREATE )
        {
            CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>( lParam );
            pThis = reinterpret_cast<DERIVED_TYPE*>( pCreate->lpCreateParams );
            SetWindowLongPtr( hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>( pThis ) );

            pThis->m_hWnd = hwnd;
        }
        else
        {
            pThis = reinterpret_cast<DERIVED_TYPE*>( GetWindowLongPtr( hwnd, GWLP_USERDATA ) );
        }

        if( pThis )
        {
            return pThis->handleMessage( uMsg, wParam, lParam );
        }
        else
        {
            return DefWindowProc( hwnd, uMsg, wParam, lParam );
        }
    }

    //--------------------------------------------------------------------------------------------------------------

    template <class DERIVED_TYPE>
    BOOL BaseWindow<DERIVED_TYPE>::create( PCWSTR lpWindowName, 
                                           DWORD dwStyle,
                                           DWORD dwExStyle, 
                                           int x, 
                                           int y, 
                                           int nWidth,
                                           int nHeight,
                                           HWND hWndParent,
                                           HMENU hMenu )
    {
        if( !registerClass() )
        {
            return false;
        }

        m_hWnd = CreateWindowEx( dwExStyle,
                                 className(),
                                 lpWindowName,
                                 dwStyle,
                                 x,
                                 y,
                                 nWidth,
                                 nHeight,
                                 hWndParent,
                                 hMenu,
                                 GetModuleHandle( nullptr ),
                                 this );

        return ( m_hWnd ? true : false );
    }

    //--------------------------------------------------------------------------------------------------------------

    template<class DERIVED_TYPE>
    inline BOOL BaseWindow<DERIVED_TYPE>::show( int nCmdShow )
    {
        return ShowWindow( m_hWnd, nCmdShow );
    }

    //--------------------------------------------------------------------------------------------------------------

    template<class DERIVED_TYPE>
    inline BOOL BaseWindow<DERIVED_TYPE>::update()
    {
        return UpdateWindow( m_hWnd );
    }


    //--------------------------------------------------------------------------------------------------------------

    template <class DERIVED_TYPE>
    inline HWND BaseWindow<DERIVED_TYPE>::getWindowHandle() const
    {
        return m_hWnd;
    }

    //--------------------------------------------------------------------------------------------------------------

    template <class DERIVED_TYPE>
    inline void BaseWindow<DERIVED_TYPE>::initWndClass(
        WNDCLASS& outWndClass ) const
    {
        outWndClass.hInstance = GetModuleHandle( nullptr );
        outWndClass.lpfnWndProc = DERIVED_TYPE::windowProc;
        outWndClass.style = CS_HREDRAW | CS_VREDRAW;
        outWndClass.hbrBackground = reinterpret_cast<HBRUSH>( COLOR_WINDOW + 1 );
        outWndClass.lpszClassName = className();
    }

    //--------------------------------------------------------------------------------------------------------------

    template <class DERIVED_TYPE>
    inline ATOM BaseWindow<DERIVED_TYPE>::registerClass() const
    {
        WNDCLASS wc{};
        initWndClass( wc );

        return RegisterClass( &wc );
    }

}  // namespace PaintLite