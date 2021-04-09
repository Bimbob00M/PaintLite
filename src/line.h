#pragma once

#include "drawing_tool.h"

namespace PaintLite
{
    class Line : public DrawingTool
    {
    public:
        Line() noexcept;
        Line( const Gdiplus::Color& color, const Gdiplus::REAL width = 1.F ) noexcept;
        Line( const Gdiplus::Pen& pen ) noexcept;

        void draw( Gdiplus::Graphics& graphics, bool shiftPressed = false ) const noexcept override;
    };
}