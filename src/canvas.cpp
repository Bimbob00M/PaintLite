#include "canvas.h"

using namespace Gdiplus;

namespace PaintLite
{
    Canvas::Canvas() noexcept
    {}

    Canvas::Canvas( const int width,
                    const int height,
                    const Color& bkgndColor,
                    const PixelFormat format ) :
        m_pBitmap{ new Bitmap( width, height, format ) },
        m_bkgndColor{ bkgndColor }
    {
        clear( m_bkgndColor );
    }

    Canvas::Canvas( const std::wstring& fileName, const Color& bkgndColor, bool useEmbeddedColorManagment ) :
        m_pBitmap{ new Bitmap( fileName.c_str(), useEmbeddedColorManagment ) },
        m_bkgndColor{ bkgndColor }
    {
        clear( m_bkgndColor );
    }

    Canvas::~Canvas()
    {
        delete m_pBitmap;
        m_pBitmap = nullptr;
    }

    Status Canvas::resize( const UINT width, const UINT height, const bool expandOnly )
    {
        if( !expandOnly ||
            width > m_pBitmap->GetWidth() ||
            height > m_pBitmap->GetHeight() )
        {
            Bitmap* pNewBitmap = new Bitmap( width, height, m_pBitmap->GetPixelFormat() );

            if( !pNewBitmap )
                return pNewBitmap->GetLastStatus();

            Graphics g( pNewBitmap );
            g.Clear( m_bkgndColor );
            g.DrawImage( m_pBitmap, 0, 0 );

            delete m_pBitmap;

            m_pBitmap = pNewBitmap;
            pNewBitmap = nullptr;
        }
        return Status::Ok;
    }
}
