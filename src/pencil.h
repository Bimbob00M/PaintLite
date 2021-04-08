#pragma once

#include "drawing_tool.h"

namespace PaintLite
{
    class Pencil : public DrawingTool
    {
    public:
        Pencil() noexcept;
        Pencil( const Gdiplus::Color& color, const Gdiplus::REAL width = 1.F ) noexcept;
        Pencil( const Gdiplus::Pen& pen ) noexcept;

        void draw( Gdiplus::Graphics& graphics, bool shiftPressed = false ) const noexcept override;

        std::array<int, 4> getPossibleThickness() const override;                
    };
}