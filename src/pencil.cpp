#include "src/pencil.h"

using namespace Gdiplus;

namespace PaintLite
{
    Pencil::Pencil() noexcept :
        DrawingTool{}
    {}

    Pencil::Pencil( const Gdiplus::Color & color, const Gdiplus::REAL width ) noexcept :
        DrawingTool{ color, width }
    {}
    
    void Pencil::draw( Graphics& graphics ) const noexcept
    {
        graphics.DrawLine( &m_pen, m_prevMousePos, m_currMousePos );
    }

    std::array<int, 4> Pencil::getPossibleThickness() const
    {
        return { 1, 2, 3, 4 };
    }
}