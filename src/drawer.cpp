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
            m_pTool->setCurrMousePos( mousePos );
            m_pTool->draw( graphics );
            m_pTool->setPrevMousePos( mousePos );
        }
    }

    void Drawer::draw( const Gdiplus::Point& mousePos ) const noexcept
    {
        //Graphics g()
    }
}
