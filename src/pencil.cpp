#include "pencil.h"

using namespace Gdiplus;

namespace PaintLite
{
    Pencil::Pencil() noexcept
        : DrawingTool{}
    {}

    Pencil::Pencil( const Gdiplus::Color & color, const Gdiplus::REAL width ) noexcept
        : DrawingTool{ color, width }
    {}

    Pencil::Pencil( const Gdiplus::Pen& pen ) noexcept
        : DrawingTool{ pen }
    {}

    void Pencil::draw( Graphics& graphics, bool shiftPressed ) const noexcept
    {
        graphics.DrawLine( m_pen, m_prevMousePos, m_currMousePos );
    }
}