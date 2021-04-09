#include <algorithm>

#include "rectangle.h"

using namespace Gdiplus;

namespace PaintLite
{
    Rectangle::Rectangle() noexcept
        : DrawingTool{}
    {}

    Rectangle::Rectangle( const Gdiplus::Color& color, const Gdiplus::REAL width ) noexcept
        : DrawingTool{ color, width }
    {}
    
    Rectangle::Rectangle( const Gdiplus::Pen& pen ) noexcept
        : DrawingTool{ pen }
    {}

    void Rectangle::draw( Gdiplus::Graphics& graphics, bool shiftPressed ) const noexcept
    {
        int x = ( std::min )( m_startPoint.X, m_currMousePos.X );
        int y = ( std::min )( m_startPoint.Y, m_currMousePos.Y );
        int width = std::abs( m_startPoint.X - m_currMousePos.X );
        int height = std::abs( m_startPoint.Y - m_currMousePos.Y );

        if( shiftPressed && width < height )
        {
            height = width;
        }
        else if( shiftPressed )
        {
            width = height;
        }

        graphics.Clear( Color::Transparent );
        graphics.DrawRectangle( m_pen, x, y, width, height );
    }
}
