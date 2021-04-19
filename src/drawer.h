#pragma once

#include <Windows.h>
#include <gdiplus.h>

#include "canvas.h"
#include "drawing_tool.h"

namespace PaintLite
{
    class Drawer
    {
    public:
        Drawer() noexcept;
        ~Drawer() noexcept;
        
        void draw( Gdiplus::Graphics& graphics, const Gdiplus::Point& mousePos) const noexcept;
        
        void startDrawing( const Gdiplus::Point& startPoint ) noexcept;
        void finishDrawing( const Gdiplus::Point& endPoint ) noexcept;
        void cancelDrawing() noexcept;
        void trackShiftKeyState( const bool state ) noexcept { m_shiftKeyState = state; }
            
        inline bool isDrawing() const noexcept { return m_drawingFlag; }
        inline bool isCanceled() const noexcept { return m_cancelFlag; }
        
        inline void setTool( DrawingTool* pNewTool ) noexcept { m_activeTool = pNewTool; }
        inline const DrawingTool* getTool() const noexcept { return m_activeTool; }
        inline DrawingTool* getTool() noexcept { return m_activeTool; }

        inline void setMainColor( const Gdiplus::Color& newColor ) noexcept { m_mainColor = newColor; }
        inline Gdiplus::Color getMainColor() const noexcept { return m_mainColor; }

        inline void setAdditionalColor( const Gdiplus::Color& newColor ) noexcept { m_additionalColor = newColor; }
        inline Gdiplus::Color getAdditionalColor() const noexcept  { return m_additionalColor; }
                
        inline void setAdditionalColorFlag( bool state ) noexcept { m_additionalColorFlag = state; }
        inline void setFillFlag( bool state ) noexcept { m_fillFlag = state; }
        
    private:        
        DrawingTool* m_activeTool{ nullptr };

        Gdiplus::Color m_mainColor{ Gdiplus::Color::Black };
        Gdiplus::Color m_additionalColor{ Gdiplus::Color::White };
        
        bool m_drawingFlag{ false };
        bool m_shiftKeyState{ false };
        bool m_cancelFlag{ false };

        bool m_additionalColorFlag{ false }; // if true, an additional color will be used for drawing
        bool m_fillFlag{ false }; // if true, an additional color will be used to fill a shapes
    };    
}