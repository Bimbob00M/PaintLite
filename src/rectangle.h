#pragma once

#include "drawing_tool.h"

namespace PaintLite
{
    class Rectangle : public DrawingTool
    {
    public:
        Rectangle() noexcept;
        Rectangle( const Gdiplus::Color& color, const Gdiplus::REAL width = 1.F ) noexcept;
        Rectangle( const Gdiplus::Pen& pen, const Gdiplus::SolidBrush* fillBrush ) noexcept;

        void draw( Gdiplus::Graphics& graphics, bool shiftPressed = false ) const noexcept override;
    };
}