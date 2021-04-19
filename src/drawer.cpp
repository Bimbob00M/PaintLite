#include "drawer.h"

using namespace Gdiplus;

namespace PaintLite
{
    Drawer::Drawer() noexcept
    {}

    Drawer::~Drawer() noexcept
    {
        m_activeTool = nullptr;
    }

    void Drawer::draw( Graphics& graphics, const Gdiplus::Point& mousePos ) const noexcept
    {
        if( m_activeTool )
        {
            if( m_additionalColorFlag )
                m_activeTool->getPen().SetColor( m_additionalColor );
            else
                m_activeTool->getPen().SetColor( m_mainColor );
            
            if( auto brush = m_activeTool->getBrush() )
            {
                if( m_fillFlag )
                    brush->SetColor( m_additionalColor );
                else
                    brush->SetColor( Color::Transparent );
            }

            m_activeTool->setCurrMousePos( mousePos );
            m_activeTool->draw( graphics, m_shiftKeyState );
            m_activeTool->setPrevMousePos( mousePos );
        }
    }
    
    void Drawer::startDrawing( const Gdiplus::Point& startPoint ) noexcept
    {
        if( m_activeTool )
        {
            m_activeTool->setStartPoint( startPoint );
            m_activeTool->setCurrMousePos( startPoint );
            m_activeTool->setPrevMousePos( startPoint );
            m_drawingFlag = true;
            m_cancelFlag = false;
        }
    }

    void Drawer::finishDrawing( const Gdiplus::Point& endPoint ) noexcept
    {
        if( m_activeTool )
        {
            m_activeTool->setEndPoint( endPoint );
            m_drawingFlag = false;
            m_shiftKeyState = false;
        }
    }

    void Drawer::cancelDrawing() noexcept
    {
        m_drawingFlag = false;
        m_shiftKeyState = false;
        m_cancelFlag = true;        
    }

}
