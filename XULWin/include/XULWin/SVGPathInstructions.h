#ifndef PATHINSTRUCTIONS_H_INCLUDED
#define PATHINSTRUCTIONS_H_INCLUDED


#include "XULWin/Point.h"
#include <utility>


namespace XULWin
{

    class PathInstruction
    {
    public:
        enum Type
        {
            MoveTo,                         // M
            LineTo,                         // L
            HorizontalLineTo,               // H
            VerticalLineTo,                 // V
            CurveTo,                        // C
            SmoothCurveTo,                  // S
            QuadraticBelzierCurve,          // Q
            SmoothQuadraticBelzierCurveTo,  // T
            EllipticalArc,                  // A
            ClosePath                       // Z
        };

        enum Positioning
        {
            Relative,
            Absolute
        };

        PathInstruction(Type inType, Positioning inPositioning, const PointFs & inPoints);

        Type tagName() const;

        size_t numPoints() const;

        const PointF & getPoint(size_t inIdx) const;

        PointF getAbsolutePoint(const PointF & inRef, size_t inIdx) const;

        Positioning positioning() const;

        const PointFs & points() const;

        PointFs & points();

    private:
        Type mType;
        PointFs mPoints;
        Positioning mPositioning;
    };

    typedef std::vector<PathInstruction> SVGPathInstructions;

} // namespace XULWin


#endif // PATHINSTRUCTIONS_H_INCLUDED
