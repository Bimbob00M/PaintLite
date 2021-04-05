#include "src/drawing_tool.h"

using namespace Gdiplus;

namespace PaintLite
{
    DrawingTool::DrawingTool() noexcept :
        m_pen{ Color::Black },
        m_thickness{ 1.F }
    {}
    DrawingTool::DrawingTool( const Gdiplus::Color& color, const Gdiplus::REAL width ) noexcept :
        m_pen{ color, width },
        m_thickness{ width }
    {}
    DrawingTool::DrawingTool( const Gdiplus::Brush* brush, const Gdiplus::REAL width ) noexcept :
        m_pen{ brush, width },
        m_thickness{ width }
    {}
}