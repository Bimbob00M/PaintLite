#pragma once

#include <memory>
#include <vector>

#include <src/drawing_tool.h>

namespace PaintLite
{
    class DrawingToolkit
    {
    public:
        DrawingToolkit();

    private:
        std::vector<std::unique_ptr<DrawingTool>> m_tools;
    };
}