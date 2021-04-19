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
        DrawingTool( const Gdiplus::Pen& pen, const Gdiplus::SolidBrush* fillBrush = nullptr ) noexcept;
        
        DrawingTool( const DrawingTool& tool ) noexcept;
        DrawingTool& operator=( const DrawingTool& tool ) noexcept;

        virtual ~DrawingTool();

        virtual void draw( Gdiplus::Graphics& graphics, bool shiftPressed = false ) const noexcept = 0;
        virtual std::array<int, 4> getPossibleThickness() const { return { 1, 3, 5, 8 }; }
                
        inline const Gdiplus::Pen& getPen() const noexcept { return *m_pen; }
        inline Gdiplus::Pen& getPen() noexcept { return *m_pen; }

        inline void setFillBrush( const Gdiplus::SolidBrush& brush ) noexcept;

        inline const Gdiplus::SolidBrush* getBrush() const noexcept { return m_fillBrush; }
        inline Gdiplus::SolidBrush* getBrush() noexcept { return m_fillBrush; }

        inline void setStartPoint( const Gdiplus::Point& startPoint ) noexcept { m_startPoint = startPoint; }
        inline void setEndPoint( const Gdiplus::Point& endPoint ) noexcept { m_endPoint = endPoint; }

        inline void setCurrMousePos( const Gdiplus::Point& currPos ) noexcept {  m_currMousePos = currPos; }
        inline void setPrevMousePos( const Gdiplus::Point& prevPos ) noexcept { m_prevMousePos = prevPos; }

        void setThickness( const Gdiplus::REAL thickness ) noexcept;

        inline Gdiplus::Point getStartPoint() const noexcept { return m_startPoint; }
        inline Gdiplus::Point getEndPoint() const noexcept { return m_endPoint; }

        inline Gdiplus::Point getCurrMousePos() const noexcept { return m_currMousePos; }
        inline Gdiplus::Point getPrevMousePos() const noexcept { return m_prevMousePos; }

        inline Gdiplus::REAL getThickness() const noexcept { return m_thickness; }

    protected:
        Gdiplus::Pen* m_pen{ nullptr };
        Gdiplus::SolidBrush* m_fillBrush{ nullptr };

        Gdiplus::Point m_startPoint{};
        Gdiplus::Point m_endPoint{};

        Gdiplus::Point m_currMousePos{};
        Gdiplus::Point m_prevMousePos{};

        Gdiplus::REAL m_thickness{ 1.F };
    };        
}

