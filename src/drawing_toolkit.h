#pragma once

#include <memory>
#include <unordered_map>

#include "drawing_tool.h"
#include "global.h"

namespace PaintLite
{
    using TToolsSet = std::unordered_map<EToolbarIDs, std::unique_ptr<DrawingTool>>;

    class DrawingToolkit
    {
    public:
        DrawingToolkit();
               
        const TToolsSet& getTools() const noexcept { return m_tools; }
        TToolsSet& getTools() noexcept { return m_tools; }

        const DrawingTool* getTool( EToolbarIDs key ) const noexcept;
        DrawingTool* getTool( EToolbarIDs key ) noexcept;
        
    private:
        TToolsSet m_tools;
    };
}