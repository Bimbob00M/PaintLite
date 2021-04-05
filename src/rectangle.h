#pragma once

#include "src/drawing_tool.h"

namespace PaintLite
{
    class Rectangle : public DrawingTool
    {
    public:
        Rectangle() noexcept;
        Rectangle( const Gdiplus::Color& color, const Gdiplus::REAL width = 1.F ) noexcept;

        void draw( Gdiplus::Graphics& graphics ) const noexcept override;

        std::array<int, 4> getPossibleThickness() const override;
    };
}