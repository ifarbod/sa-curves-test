#include "curves.hpp"

// #define USE_CUSTOM_IMPL 1

// fn @ 0x43C880 (finished)
f32 CCurves::CalcCorrectedDist(f32 Current, f32 Total, f32 SpeedVariation, f32* pInterPol)
{
#ifdef USE_CUSTOM_IMPL
    if (Total >= 0.00001f)
    {
        f32 AverageSpeed = (Total / TWO_PI) * SpeedVariation;
        f32 CorrectedDist = (((SpeedVariation * -2.0f) + 2.0f) * 0.5f * Current) +
                            (AverageSpeed * CMaths::Sin((Current * TWO_PI) / Total));
        *pInterPol = 0.5f - (CMaths::Cos((Current / Total) * PI) * 0.5f);
        return CorrectedDist;
    }

    *pInterPol = 0.5f;
    return 0.0f;
#else
    return Call<0x43C880, f32>(Current, Total, SpeedVariation, pInterPol);
#endif
}

// fn @ 0x43C900
void CCurves::CalcCurvePoint(const CVector& startCoors, const CVector& endCoors, const CVector& startDir,
    const CVector& endDir, f32 Time, i32 TraverselTimeInMillis, CVector& resultCoor,
    CVector& resultSpeed)  // TODO(iFarbod): TraversalTimeInMillis, param name has typo
{
#ifdef USE_CUSTOM_IMPL
        // This function calculates a point on a smooth curve between two positions with direction vectors.
    // The curve consists of straight segments and a bend connecting them for natural-looking movement.

    // Normalize time parameter to ensure calculations remain within valid range
    f32 OurTime = VCLAMP(0.0f, 1.0f, Time);

    // Get speed adjustment factor needed for realistic bends (slower in curves, faster on straights)
    f32 SpeedVariation = CalcSpeedVariationInBend(startCoors, endCoors, startDir.x, startDir.y, endDir.x, endDir.y);

    // Find where the ray from start position would intersect with end ray
    f32 DistToPoint1 = DistForLineToCrossOtherLine(
        startCoors.x, startCoors.y, startDir.x, startDir.y, endCoors.x, endCoors.y, endDir.x, endDir.y);

    // Find where the ray from end position would intersect with start ray (negative because direction is flipped)
    f32 DistToPoint2 = -DistForLineToCrossOtherLine(
        endCoors.x, endCoors.y, endDir.x, endDir.y, startCoors.x, startCoors.y, startDir.x, startDir.y);

    if (DistToPoint1 <= 0.0f || DistToPoint2 <= 0.0f)
    {
        // If rays don't intersect properly, fall back to a simpler curved path approximation
        // This happens when the directions would never cross or are almost parallel

        const f32 StraightDist = (startCoors - endCoors).Magnitude2D();

        // Calculate path distances adjusted for speed variation
        f32 BendDist = StraightDist / (1.0f - SpeedVariation);
        f32 BendDist_Time = BendDist * OurTime;
        f32 CurrentDist_Time = CalcCorrectedDist(BendDist_Time, BendDist, SpeedVariation, &SpeedVariation);

        // Calculate position along start ray
        const CVector startPoint = startCoors + (startDir * CurrentDist_Time);

        // Calculate position along end ray
        const f32 distDiff = CurrentDist_Time - StraightDist;
        const CVector endPoint = endCoors + (endDir * distDiff);

        // Blend between the two projected positions based on speed variation
        f32 Interpol = 1.0f - SpeedVariation;
        f32 StraightDist1 = 0.0f;
        f32 StraightDist2 = 0.0f;

        resultCoor = (startPoint * Interpol) + (endPoint * SpeedVariation);

        // Zero speed for this special case (likely a placeholder as this value isn't used)
        f32 TotalDist_Time = 0.0f;
        const f32 timeScale = static_cast<f32>(TraverselTimeInMillis) * 0.001f;
        const f32 t1 = 1.0f - OurTime;

        resultSpeed = ((endDir * OurTime) + (startDir * t1)) * (TotalDist_Time / timeScale);
        resultSpeed.z = 0.0f;

        return;
    }

    // For properly intersecting rays, create a three-segment path:
    // 1. Straight segment from start
    // 2. Curved bend in the middle
    // 3. Straight segment to end

    // Limit how sharp the bend can be for natural movement
    f32 BendDistOneSegment = CMaths::Min(CMaths::Min(DistToPoint1, DistToPoint2), 5.0f);

    // Calculate the three segment lengths
    f32 StraightDist1 = DistToPoint1 - BendDistOneSegment;
    f32 StraightDist2 = DistToPoint2 - BendDistOneSegment;
    f32 BendDist = BendDistOneSegment * 2.0f;
    f32 TotalDist_Time = StraightDist1 + BendDist + StraightDist2;

    const f32 distanceAtTime = TotalDist_Time * OurTime;

    if (distanceAtTime < StraightDist1)
    {
        // Position is on the first straight segment (linear interpolation from start)
        resultCoor = startCoors + (startDir * distanceAtTime);
    }
    else if (distanceAtTime > (StraightDist1 + BendDist))
    {
        // Position is on the final straight segment (linear interpolation to end)
        const f32 secondSegmentDist = distanceAtTime - (StraightDist1 + BendDist);
        resultCoor = endCoors + (endDir * secondSegmentDist);
    }
    else
    {
        // Position is in the curved bend section - requires double interpolation
        // First interpolate through the bend progress, then between the influenced points
        f32 BendInter = (distanceAtTime - StraightDist1) / BendDist;

        // Find the start point of the bend
        CVector BendStartCoors = startCoors + (startDir * StraightDist1);

        // Find the end point of the bend
        CVector BendEndCoors = endCoors - (endDir * StraightDist2);

        // Create smooth transition by double-interpolating between the bend points
        const f32 oneMinusBendInter = 1.0f - BendInter;

        // Create influence points that extend outward from the bend endpoints
        const CVector startInfluence = BendStartCoors + (startDir * (BendDistOneSegment * BendInter));
        const CVector endInfluence = BendEndCoors - (endDir * (BendDistOneSegment * oneMinusBendInter));

        // Blend between influence points to create curved path
        resultCoor = (startInfluence * oneMinusBendInter) + (endInfluence * BendInter);
    }

    // Calculate velocity based on blend of start/end directions and total path length
    const f32 timeScale = static_cast<f32>(TraverselTimeInMillis) * 0.001f;
    const f32 t1 = 1.0f - OurTime;

    resultSpeed = ((endDir * OurTime) + (startDir * t1)) * (TotalDist_Time / timeScale);
    resultSpeed.z = 0.0f;
#else
    Call<0x43C900>(&startCoors, &endCoors, &startDir, &endDir, Time, TraverselTimeInMillis, &resultCoor, &resultSpeed);
#endif
}

// fn @ 0x43C710 ?CalcSpeedScaleFactor@CCurves@@SAMABVCVector@@0MMMM@Z (finished)
f32 CCurves::CalcSpeedScaleFactor(
    const CVector& startCoors, const CVector& endCoors, f32 StartDirX, f32 StartDirY, f32 EndDirX, f32 EndDirY)
{
#ifdef USE_CUSTOM_IMPL
    f32 SpeedVariation =
        CCurves::CalcSpeedVariationInBend(startCoors, endCoors, StartDirX, StartDirY, EndDirX, EndDirY);

    // Calculate intersection distances using helper function
    f32 DistToPoint1 = CCurves::DistForLineToCrossOtherLine(
        startCoors.x, startCoors.y, StartDirX, StartDirY, endCoors.x, endCoors.y, EndDirX, EndDirY);

    // yes the negation (-) is intentional
    f32 DistToPoint2 = -CCurves::DistForLineToCrossOtherLine(
        endCoors.x, endCoors.y, EndDirX, EndDirY, startCoors.x, startCoors.y, StartDirX, StartDirY);

    if (DistToPoint1 <= 0.0f || DistToPoint2 <= 0.0f)
    {
        // Calculate straight line distance
        CVector diff = startCoors - endCoors;
        f32 StraightDist1 = diff.x;
        f32 StraightDist2 = diff.y;
        f32 StraightDist = diff.Magnitude2D();
        return StraightDist / (1.0f - SpeedVariation);
    }

    // clamp bend distance to 5.0f
    f32 BendDistOneSegment = CMaths::Min(CMaths::Min(DistToPoint1, DistToPoint2), 5.0f);

    const f32 StartDirYa = DistToPoint1 - BendDistOneSegment;
    const f32 EndDirYa = DistToPoint2 - BendDistOneSegment;

    f32 BendDist = BendDistOneSegment * 2.0f + StartDirYa + EndDirYa;
    f32 TotalDist_Time = BendDist;
    return TotalDist_Time;
#else
    return Call<0x43C710, f32>(&startCoors, &endCoors, StartDirX, StartDirY, EndDirX, EndDirY);
#endif
}

// fn @ 0x43C660 ?CalcSpeedVariationInBend@CCurves@@SAMABVCVector@@0MMMM@Z (finished)
f32 CCurves::CalcSpeedVariationInBend(
    const CVector& startCoors, const CVector& endCoors, f32 StartDirX, f32 StartDirY, f32 EndDirX, f32 EndDirY)
{
#ifdef USE_CUSTOM_IMPL
    f32 ReturnVal = 0.0f;
    f32 DotProduct = StartDirX * EndDirX + StartDirY * EndDirY;

    if (DotProduct <= 0.0f)
    {
        // If the dot product is <= 0, return a constant value (1/3)
        ReturnVal = 1.0f / 3.0f;
    }
    else if (DotProduct <= 0.7f)
    {
        // If the dot product is <= 0.7, interpolate the return value
        ReturnVal = (1.0f - (DotProduct / 0.7f)) * (1.0f / 3.0f);
    }
    else
    {
        // Calculate the distance from the start point to the mathematical line defined by the end point and direction
        f32 DistToLine =
            CCollision::DistToMathematicalLine2D(endCoors.x, endCoors.y, EndDirX, EndDirY, startCoors.x, startCoors.y);

        // Calculate the straight-line distance between the start and end points
        f32 StraightDist = (startCoors - endCoors).Magnitude2D();

        // Normalize the distance to the line by the straight-line distance
        ReturnVal = (DistToLine / StraightDist) * (1.0f / 3.0f);
    }

    return ReturnVal;
#else
    return Call<0x43C660, f32>(&startCoors, &endCoors, StartDirX, StartDirY, EndDirX, EndDirY);
#endif
}

// fn @ 0x43C610 (finished)
f32 CCurves::DistForLineToCrossOtherLine(f32 LineBaseX, f32 LineBaseY, f32 LineDirX, f32 LineDirY, f32 OtherLineBaseX,
    f32 OtherLineBaseY, f32 OtherLineDirX, f32 OtherLineDirY)
{
#ifdef USE_CUSTOM_IMPL
    f32 Dir = LineDirX * OtherLineDirY - LineDirY * OtherLineDirX;

    if (Dir == 0.0f)
    {
        return -1.0f;  // Lines are parallel, no intersection
    }

    f32 Dist = ((LineBaseX - OtherLineBaseX) * OtherLineDirY) - ((LineBaseY - OtherLineBaseY) * OtherLineDirX);
    f32 DistOfCrossing = -Dist / Dir;

    return DistOfCrossing;
#else
    return Call<0x43C610, f32>(
        LineBaseX, LineBaseY, LineDirX, LineDirY, OtherLineBaseX, OtherLineBaseY, OtherLineDirX, OtherLineDirY);
#endif
}
