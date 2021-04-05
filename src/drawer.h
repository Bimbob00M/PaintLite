#pragma once

#include <Windows.h>
#include <gdiplus.h>

#include "src/canvas.h"
#include "src/drawing_tool.h"

namespace PaintLite
{
    class Drawer
    {
    public:
        Drawer() noexcept;
        ~Drawer() noexcept;
        
        void draw( Gdiplus::Graphics& graphics, const Gdiplus::Point& mousePos ) const noexcept;
        void draw( const Gdiplus::Point& mousePos ) const noexcept;
        
        inline void startDrawing( const Gdiplus::Point& startPoint ) noexcept;
        inline void finishDrawing( const Gdiplus::Point& endPoint ) noexcept;
    
        inline bool isDrawing() const noexcept;
        
        inline void setTool( DrawingTool* pNewTool ) noexcept;

        inline void setMainColor( const Gdiplus::Color& newColor ) noexcept;
        inline void setAdditionalColor( const Gdiplus::Color& newColor ) noexcept;

        inline const DrawingTool* getTool() const noexcept;
        inline DrawingTool* getTool() noexcept;
        
        inline Gdiplus::Color getMainColor() const noexcept;
        inline Gdiplus::Color getAdditionalColor() const noexcept;
        
    private:        
        DrawingTool* m_pTool{ nullptr };

        Gdiplus::Color m_mainColor{ Gdiplus::Color::Black };
        Gdiplus::Color m_additionalColor{ Gdiplus::Color::White };
        
        bool drawingFlag{ false };
    };
    
    //*****************************************************************************************
    
    inline void Drawer::startDrawing( const Gdiplus::Point& startPoint ) noexcept
    {
        if( m_pTool )
        {
            m_pTool->setStartPoint( startPoint );
            m_pTool->setCurrMousePos( startPoint );
            m_pTool->setPrevMousePos( startPoint );
            drawingFlag = true;
        }
    }

    inline void Drawer::finishDrawing( const Gdiplus::Point& endPoint ) noexcept
    {
        if( m_pTool )
        {
            m_pTool->setEndPoint( endPoint );
            drawingFlag = false;
        }
    }

    inline bool Drawer::isDrawing() const noexcept
    {
        return drawingFlag;
    }
    
    inline void Drawer::setTool( DrawingTool* pNewTool ) noexcept
    {
        m_pTool = pNewTool;
    }

    inline void Drawer::setMainColor( const Gdiplus::Color& newColor ) noexcept
    {
        m_mainColor = newColor;
    }

    inline void Drawer::setAdditionalColor( const Gdiplus::Color & newColor ) noexcept
    {
        m_additionalColor = newColor;
    }

    inline const DrawingTool* Drawer::getTool() const noexcept
    {
        return m_pTool;
    }

    inline DrawingTool* Drawer::getTool() noexcept
    {
        return m_pTool;
    }

    inline Gdiplus::Color Drawer::getMainColor() const noexcept
    {
        return m_mainColor;
    }

    inline Gdiplus::Color Drawer::getAdditionalColor() const noexcept
    {
        return m_additionalColor;
    }
}