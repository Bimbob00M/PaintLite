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
    
    Ellipse::Ellipse( const Gdiplus::Pen& pen ) noexcept
        : DrawingTool{ pen }
    {}

    void Ellipse::draw( Gdiplus::Graphics& graphics, bool shiftPressed ) const noexcept
    {
        int x = ( std::min )( m_startPoint.X, m_currMousePos.X );
        int y = ( std::min )( m_startPoint.Y, m_currMousePos.Y );
        int width = std::abs( m_startPoint.X - m_currMousePos.X );
        int height = std::abs( m_startPoint.Y - m_currMousePos.Y );
        
        if( shiftPressed && width < height )
            height = width;
        else if( shiftPressed )
            width = height;
        
        graphics.Clear( Color::Transparent );
        graphics.DrawEllipse( m_pen, x, y, width, height );
    }

    std::array<int, 4> Ellipse::getPossibleThickness() const
    {
        return { 1, 3, 5, 8 };
    }
}