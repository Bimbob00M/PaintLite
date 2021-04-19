#pragma once

#include "drawing_tool.h"

namespace PaintLite
{
    class Ellipse : public DrawingTool
    {
    public:
        Ellipse() noexcept;
        Ellipse( const Gdiplus::Color& color, const Gdiplus::REAL width = 1.F ) noexcept;
        Ellipse( const Gdiplus::Pen& pen, const Gdiplus::SolidBrush* fillBrush ) noexcept;

        void draw( Gdiplus::Graphics& graphics, bool shiftPressed = false ) const noexcept override;
    };
}