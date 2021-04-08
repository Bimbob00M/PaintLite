#include "drawing_toolkit.h"
#include "ellipse.h"
#include "line.h"
#include "pencil.h"
#include "rectangle.h"
#include "eraser.h"

namespace PaintLite
{
    DrawingToolkit::DrawingToolkit()
    {        
        using namespace Gdiplus;

        Pen defaultPen( Color::Black );
        defaultPen.SetLineCap( LineCapRound, LineCapRound, DashCapFlat );

        m_tools[eTID_PENCIL] = std::make_unique<Pencil>( Pencil( defaultPen ) );
        m_tools[eTID_LINE] = std::make_unique<Line>( Line( defaultPen ) );
        m_tools[eTID_RECT] = std::make_unique<Rectangle>( Rectangle( defaultPen ) );
        m_tools[eTID_ELLIPSE] = std::make_unique<Ellipse>( Ellipse( defaultPen ) );
        m_tools[eTID_ERASER] = std::make_unique<Eraser>( Eraser( defaultPen ) );
    }
    
    const DrawingTool* DrawingToolkit::getTool( EToolbarIDs key ) const noexcept
    {
        auto it = m_tools.find( key );
        if( it != m_tools.end() )
        {
            return it->second.get();
        }
        return nullptr;
    }

    DrawingTool* DrawingToolkit::getTool( EToolbarIDs key ) noexcept
    {
        auto it = m_tools.find( key );
        if( it != m_tools.end() )
        {
            return it->second.get();
        }
        return nullptr;
    }
}