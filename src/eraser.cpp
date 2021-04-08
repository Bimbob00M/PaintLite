#include "eraser.h"

using namespace Gdiplus;

namespace PaintLite
{
    Eraser::Eraser() noexcept
        : DrawingTool{}
    {}

    Eraser::Eraser( const Gdiplus::Color& color, const Gdiplus::REAL width ) noexcept
        : DrawingTool{ color, width }
    {}

    Eraser::Eraser( const Gdiplus::Pen& pen ) noexcept
        : DrawingTool{ pen }
    {}

    void Eraser::draw( Graphics& graphics, bool shiftPressed ) const noexcept
    {
        graphics.DrawLine( m_pen, m_prevMousePos, m_currMousePos );
    }

    std::array<int, 4> Eraser::getPossibleThickness() const
    {
        return { 4, 6, 8, 10 };
    }
}