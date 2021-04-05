#include <algorithm>

#include "src/rectangle.h"

using namespace Gdiplus;

namespace PaintLite
{
    Rectangle::Rectangle() noexcept :
        DrawingTool{}
    {}

    Rectangle::Rectangle( const Gdiplus::Color& color, const Gdiplus::REAL width ) noexcept :
        DrawingTool{ color, width }
    {}

    void Rectangle::draw( Gdiplus::Graphics& graphics ) const noexcept
    {
        int x = ( std::min )( m_startPoint.X, m_currMousePos.X );
        int y = ( std::min )( m_startPoint.Y, m_currMousePos.Y );
        int width = std::abs( m_startPoint.X - m_currMousePos.X );
        int height = std::abs( m_startPoint.Y - m_currMousePos.Y );

        graphics.Clear( Color::Transparent );
        graphics.DrawRectangle( &m_pen, x, y, width, height );
    }

    std::array<int, 4> Rectangle::getPossibleThickness() const
    {
        return { 1, 3, 5, 8 };
    }
}
