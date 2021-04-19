#include "ellipse.h"

using namespace Gdiplus;

namespace PaintLite
{
    Ellipse::Ellipse() noexcept
        : DrawingTool{}
    {}

    Ellipse::Ellipse( const Gdiplus::Color& color, const Gdiplus::REAL width ) noexcept
        : DrawingTool{ color, width }
    {}
    
    Ellipse::Ellipse( const Gdiplus::Pen& pen, const Gdiplus::SolidBrush* fillBrush ) noexcept
        : DrawingTool{ pen, fillBrush }
    {}

    void Ellipse::draw( Gdiplus::Graphics& graphics, bool shiftPressed ) const noexcept
    {
        int x = ( std::min )( m_startPoint.X, m_currMousePos.X );
        int y = ( std::min )( m_startPoint.Y, m_currMousePos.Y );
        int width = std::abs( m_startPoint.X - m_currMousePos.X );
        int height = std::abs( m_startPoint.Y - m_currMousePos.Y );
        
        if( shiftPressed )
        {
            if( width < height )
                height = width;
            else
                width = height;

            x = m_currMousePos.X > m_startPoint.X ? m_startPoint.X : m_startPoint.X - width;
            y = m_currMousePos.Y > m_startPoint.Y ? m_startPoint.Y : m_startPoint.Y - width;
        }
        
        graphics.Clear( Color::Transparent );

        if( m_fillBrush )
            graphics.FillEllipse( m_fillBrush, x, y, width, height );
        graphics.DrawEllipse( m_pen, x, y, width, height );
    }
}