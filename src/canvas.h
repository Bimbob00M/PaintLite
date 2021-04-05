#pragma once

#include <string>

#include <Windows.h>

#include <gdiplus.h>

namespace PaintLite
{
    class Canvas
    {
    public:
        Canvas() noexcept;

        Canvas( const int width,
                const int height, 
                const Gdiplus::Color& bkgndColor = Gdiplus::Color::White,
                const Gdiplus::PixelFormat format = PixelFormat32bppARGB );

        Canvas( const std::wstring& fileName,
                const Gdiplus::Color& bkgndColor = Gdiplus::Color::White,
                bool useEmbeddedColorManagment = false);

        virtual ~Canvas();

        Gdiplus::Status resize( const UINT width, const UINT height, const bool expandOnly = false );
        inline Gdiplus::Status clear( const Gdiplus::Color& bkgndColor = Gdiplus::Color::White ) noexcept;

        inline Gdiplus::Bitmap* get() const noexcept;
        inline Gdiplus::Rect getRect() const noexcept;

    private:
        Gdiplus::Bitmap* m_pBitmap{ nullptr };

        Gdiplus::Color m_bkgndColor{ Gdiplus::Color::White };

        Canvas( const Canvas& ) = delete;
        Canvas& operator=( const Canvas& ) = delete;

    };
    
    inline Gdiplus::Bitmap* Canvas::get() const noexcept
    {        
        return m_pBitmap;
    }

    inline Gdiplus::Rect Canvas::getRect() const noexcept
    {
        return Gdiplus::Rect( 0, 0, m_pBitmap->GetWidth(), m_pBitmap->GetWidth() );
    }

    inline Gdiplus::Status Canvas::clear( const Gdiplus::Color& bkgndColor ) noexcept
    {
        Gdiplus::Graphics g( m_pBitmap );
        return g.Clear( bkgndColor );
    }
}

