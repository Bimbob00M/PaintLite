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

    DrawingTool::DrawingTool( const Gdiplus::Pen& pen ) noexcept
        : m_pen{ pen.Clone() }
    {}

    DrawingTool::DrawingTool( const DrawingTool& tool ) noexcept
        : m_pen{ tool.m_pen->Clone() }
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
                
        m_pen = tool.m_pen->Clone();
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
        m_pen = nullptr;
    }

    void DrawingTool::setThickness( const Gdiplus::REAL thickness ) noexcept
    {
        m_thickness = thickness;
        m_pen->SetWidth( m_thickness );
    }
}