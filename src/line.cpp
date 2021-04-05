#include "line.h"

using namespace Gdiplus;

namespace PaintLite
{
    Line::Line() noexcept :
        DrawingTool{}
    {}

    Line::Line( const Gdiplus::Color & color, const Gdiplus::REAL width ) noexcept :
        DrawingTool{ color, width }
    {}
    
    void Line::draw( Gdiplus::Graphics& graphics ) const noexcept
    {
        graphics.Clear( Color::Transparent );
        graphics.DrawLine(&m_pen, m_startPoint, m_currMousePos );
    }

    std::array<int, 4> Line::getPossibleThickness() const
    {
        return { 1, 3, 5, 8 };
    }
}