#include "canvas.h"

using namespace Gdiplus;

namespace PaintLite
{
    Canvas::Canvas( const Gdiplus::Color& bkgndColor ) noexcept
        : m_bkgndColor{ bkgndColor }
    {}

    Canvas::Canvas( const int width,
                    const int height,
                    const Color& bkgndColor,
                    const PixelFormat format ) 
        : m_canvasBitmap{ new Bitmap( width, height, format ) }
        , m_bkgndColor{ bkgndColor }
    {
        clear( m_bkgndColor );
    }

    Canvas::Canvas( const std::wstring& fileName, const Color& bkgndColor, bool useEmbeddedColorManagment )
        : m_canvasBitmap{ new Bitmap( fileName.c_str(), useEmbeddedColorManagment ) }
        , m_bkgndColor{ bkgndColor }
    {
        clear( m_bkgndColor );
    }

    Canvas::Canvas( Bitmap& bitmap ) 
        : m_canvasBitmap{ new Bitmap( bitmap.GetWidth(), bitmap.GetHeight(), bitmap.GetPixelFormat() ) }
    {        
        if( m_canvasBitmap->GetLastStatus() == Ok )
        {
            Graphics g( &bitmap );
            g.DrawImage( m_canvasBitmap, 0, 0 );
        }
    }



    Canvas::~Canvas()
    {
        delete m_canvasBitmap;
        m_canvasBitmap = nullptr;
    }

    Status Canvas::resize( const UINT width, 
                           const UINT height, 
                           const bool expandOnly,
                           const int xDrawOffset,
                           const int yDrawOffset )
    {
        if( !expandOnly ||
            !m_canvasBitmap )
        {
            PixelFormat pf = m_canvasBitmap ? m_canvasBitmap->GetPixelFormat() : PixelFormat32bppPARGB;
            Bitmap* pNewBitmap = new Bitmap( width, height, pf );

            if( !pNewBitmap )
                return pNewBitmap->GetLastStatus();

            Graphics g( pNewBitmap );
            g.Clear( m_bkgndColor );
            g.DrawImage( m_canvasBitmap,
                         xDrawOffset, yDrawOffset,
                         0, 0,
                         m_canvasBitmap ? m_canvasBitmap->GetWidth() : 0,
                         m_canvasBitmap ? m_canvasBitmap->GetHeight() : 0,
                         UnitPixel );

            delete m_canvasBitmap;

            m_canvasBitmap = pNewBitmap;
            pNewBitmap = nullptr;
        }
        return Status::Ok;
    }

    Gdiplus::Status Canvas::scale( double scalingFactor )
    {
        if( !m_canvasBitmap )
            return Status::WrongState;

        PixelFormat pf = m_canvasBitmap->GetPixelFormat();
        int width = m_canvasBitmap->GetWidth();
        int height = m_canvasBitmap->GetHeight();
        Bitmap* pNewBitmap = new Bitmap( width, height, pf );

        if( !pNewBitmap )
            return pNewBitmap->GetLastStatus();

        Graphics g( pNewBitmap );
        g.Clear( m_bkgndColor );
        g.DrawImage( m_canvasBitmap, 0, 0, scalingFactor * width, scalingFactor * height );

        delete m_canvasBitmap;

        m_canvasBitmap = pNewBitmap;
        pNewBitmap = nullptr;
        
        return Status::Ok;
    }
    
    Status Canvas::clear( const Color& bkgndColor ) noexcept
    {
        Graphics g( m_canvasBitmap );
        return g.Clear( bkgndColor );
    }
}
