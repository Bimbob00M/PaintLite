#pragma once

#include <Windows.h>
#include <gdiplus.h>

#include <string>


namespace PaintLite
{
    class Canvas
    {
    public:
        Canvas( const Gdiplus::Color& bkgndColor = Gdiplus::Color::White ) noexcept;

        Canvas( const int width,
                const int height, 
                const Gdiplus::Color& bkgndColor = Gdiplus::Color::White,
                const Gdiplus::PixelFormat format = PixelFormat32bppPARGB );

        Canvas( const std::wstring& fileName,
                const Gdiplus::Color& bkgndColor = Gdiplus::Color::White,
                bool useEmbeddedColorManagment = false);

        Canvas( Gdiplus::Bitmap& bitmap );

        virtual ~Canvas();

        Gdiplus::Status resize( const UINT width, 
                                const UINT height, 
                                const bool expandOnly = false, 
                                const int xDrawOffset = 0, 
                                const int yDrawOffset = 0 );

        Gdiplus::Status scale( double scalingFactor );

        inline Gdiplus::Status clear( const Gdiplus::Color& bkgndColor = Gdiplus::Color::White ) noexcept;

        inline Gdiplus::Bitmap* get() const { return m_canvasBitmap; };
        inline Gdiplus::Rect getRect() const noexcept { return Gdiplus::Rect( 0, 0, m_canvasBitmap->GetWidth(), m_canvasBitmap->GetWidth() ); };

    private:
        Gdiplus::Bitmap* m_canvasBitmap{ nullptr };
        Gdiplus::Color m_bkgndColor{ Gdiplus::Color::White };

        Canvas( const Canvas& ) = delete;
        Canvas& operator=( const Canvas& ) = delete;

        Canvas( Canvas&& ) = delete;
        Canvas& operator=( Canvas&& ) = delete;
    };    
}

