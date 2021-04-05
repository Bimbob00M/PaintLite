#pragma once

#include<array>

#include <Windows.h>
#include <gdiplus.h>

namespace PaintLite
{
    class DrawingTool
    {
    public:
        DrawingTool() noexcept;
        DrawingTool( const Gdiplus::Color& color, const Gdiplus::REAL width = 1.F ) noexcept;
        DrawingTool( const Gdiplus::Brush* brush, const Gdiplus::REAL width = 1.F ) noexcept;
        virtual ~DrawingTool() = default;

        virtual void draw( Gdiplus::Graphics& graphics) const noexcept = 0;

        virtual std::array<int, 4> getPossibleThickness() const = 0;


        inline const Gdiplus::Pen& getPen() const noexcept;
        inline Gdiplus::Pen& getPen() noexcept;

        inline void setStartPoint( const Gdiplus::Point& startPoint ) noexcept;
        inline void setEndPoint( const Gdiplus::Point& endPoint ) noexcept;

        inline void setCurrMousePos( const Gdiplus::Point& startPoint ) noexcept;
        inline void setPrevMousePos( const Gdiplus::Point& endPoint ) noexcept;

        inline void setThickness( const Gdiplus::REAL thickness ) noexcept;

        inline Gdiplus::Point getStartPoint() const noexcept;
        inline Gdiplus::Point getEndPoint() const noexcept;

        inline Gdiplus::Point getCurrMousePos() const noexcept;
        inline Gdiplus::Point getPrevMousePos() const noexcept;

        inline Gdiplus::REAL getThickness() const noexcept;

    protected:
        Gdiplus::Pen m_pen;

        Gdiplus::Point m_startPoint{};
        Gdiplus::Point m_endPoint{};

        Gdiplus::Point m_currMousePos{};
        Gdiplus::Point m_prevMousePos{};

        Gdiplus::REAL m_thickness;
    };

    //***********************************************************************************

    inline const Gdiplus::Pen& DrawingTool::getPen() const noexcept
    {
        return m_pen;
    }

    inline Gdiplus::Pen& DrawingTool::getPen() noexcept
    {
        return m_pen;
    }

    inline void DrawingTool::setStartPoint( const Gdiplus::Point& startPoint ) noexcept
    {
        m_startPoint = startPoint;
    }

    inline void DrawingTool::setEndPoint( const Gdiplus::Point& endPoint ) noexcept
    {
        m_endPoint = endPoint;
    }

    inline void DrawingTool::setCurrMousePos( const Gdiplus::Point& currPos ) noexcept
    {
        m_currMousePos = currPos;
    }

    inline void DrawingTool::setPrevMousePos( const Gdiplus::Point& prevPos ) noexcept
    {
        m_prevMousePos = prevPos;
    }

    inline void DrawingTool::setThickness( const Gdiplus::REAL thickness ) noexcept
    {
        m_thickness = thickness;
    }

    inline Gdiplus::Point DrawingTool::getStartPoint() const noexcept
    {
        return m_startPoint;
    }

    inline Gdiplus::Point DrawingTool::getEndPoint() const noexcept
    {
        return m_endPoint;
    }

    inline Gdiplus::Point DrawingTool::getCurrMousePos() const noexcept
    {
        return m_currMousePos;
    }

    inline Gdiplus::Point DrawingTool::getPrevMousePos() const noexcept
    {
        return m_prevMousePos;
    }
    inline Gdiplus::REAL DrawingTool::getThickness() const noexcept
    {
        return m_thickness;
    }
}

