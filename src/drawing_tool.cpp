#include "drawing_tool.h"

using namespace Gdiplus;

namespace PaintLite
{
    DrawingTool::DrawingTool() noexcept
        : m_pen{ new Pen( Color::Black ) }
    {}

    DrawingTool::DrawingTool( const Gdiplus::Color& color, const Gdiplus::REAL width ) noexcept
        : m_pen{ new Pen( color, width ) }
        , m_thickness{ width }
    {}

    DrawingTool::DrawingTool( const Gdiplus::Brush* brush, const Gdiplus::REAL width ) noexcept
        : m_pen{ new Pen( brush, width) }
        , m_thickness{ width }
    {}

    DrawingTool::DrawingTool( const Gdiplus::Pen& pen, const Gdiplus::SolidBrush* fillBrush ) noexcept
        : m_pen{ pen.Clone() }
        , m_fillBrush{ fillBrush ? static_cast<SolidBrush*>( fillBrush->Clone() ) : nullptr }
    {}

    DrawingTool::DrawingTool( const DrawingTool& tool ) noexcept
        : m_pen{ tool.m_pen->Clone() }
        , m_fillBrush{ tool.m_fillBrush ? static_cast<SolidBrush*>( tool.m_fillBrush->Clone() ) : nullptr }
        , m_startPoint{ tool.m_startPoint }
        , m_endPoint{ tool.m_endPoint }
        , m_currMousePos{ tool.m_currMousePos }
        , m_prevMousePos{ tool.m_prevMousePos }
        , m_thickness{ tool.m_thickness }
    {}

    DrawingTool& DrawingTool::operator=( const DrawingTool& tool ) noexcept
    {
        if( &tool == this )
            return *this;
         
        delete m_pen;
        m_pen = tool.m_pen->Clone();
        
        delete m_fillBrush;
        m_fillBrush = tool.m_fillBrush ? static_cast<SolidBrush*>( tool.m_fillBrush->Clone() ) : nullptr;

        m_startPoint = tool.m_startPoint;
        m_endPoint = tool.m_endPoint;
        m_currMousePos = tool.m_currMousePos;
        m_prevMousePos = tool.m_prevMousePos;
        m_thickness = tool.m_thickness;
        
        return *this;
    }

    DrawingTool::~DrawingTool()
    {
        delete m_pen;
        delete m_fillBrush;

        m_pen = nullptr;
        m_fillBrush = nullptr;
    }

    void DrawingTool::setFillBrush( const Gdiplus::SolidBrush& brush ) noexcept
    {
        delete m_fillBrush;
        m_fillBrush = static_cast<SolidBrush*>( brush.Clone() );
    }

    void DrawingTool::setThickness( const Gdiplus::REAL thickness ) noexcept
    {
        m_thickness = thickness;
        m_pen->SetWidth( m_thickness );
    }
}