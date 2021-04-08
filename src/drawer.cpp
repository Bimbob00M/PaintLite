#include "drawer.h"

using namespace Gdiplus;

namespace PaintLite
{
    Drawer::Drawer() noexcept
    {}

    Drawer::~Drawer() noexcept
    {
        m_pTool = nullptr;
    }

    void Drawer::draw( Graphics& graphics, const Gdiplus::Point& mousePos ) const noexcept
    {
        if( m_pTool )
        {
            if( m_useAdditionalColorFlag )
                m_pTool->getPen().SetColor( m_additionalColor );
            else
                m_pTool->getPen().SetColor( m_mainColor );

            m_pTool->setCurrMousePos( mousePos );
            m_pTool->draw( graphics, m_shiftKeyState );
            m_pTool->setPrevMousePos( mousePos );
        }
    }
    
    void Drawer::startDrawing( const Gdiplus::Point& startPoint ) noexcept
    {
        if( m_pTool )
        {
            m_pTool->setStartPoint( startPoint );
            m_pTool->setCurrMousePos( startPoint );
            m_pTool->setPrevMousePos( startPoint );
            m_drawingFlag = true;
            m_cancelFlag = false;
        }
    }

    void Drawer::finishDrawing( const Gdiplus::Point& endPoint ) noexcept
    {
        if( m_pTool )
        {
            m_pTool->setEndPoint( endPoint );
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
