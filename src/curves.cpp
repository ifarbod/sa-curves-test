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
    // Clamp time to [0, 1]
    if (Time > 1.0f)
    {
        Time = 1.0f;
    }
    else if (Time < 0.0f)
    {
        Time = 0.0f;
    }

    // Calculate speed variation in the bend
    f32 SpeedVariation = CalcSpeedVariationInBend(startCoors, endCoors, startDir.x, startDir.y, endDir.x, endDir.y);

    // Calculate distances to intersection points
    f32 DistToPoint1 = DistForLineToCrossOtherLine(
        startCoors.x, startCoors.y, startDir.x, startDir.y, endCoors.x, endCoors.y, endDir.x, endDir.y);

    f32 DistToPoint2 = DistForLineToCrossOtherLine(
        endCoors.x, endCoors.y, endDir.x, endDir.y, startCoors.x, startCoors.y, startDir.x, startDir.y);

    // Variables for intermediate calculations
    f32 BendDist = 0.0f;
    f32 BendDist_Time = 0.0f;
    f32 CurrentDist_Time = 0.0f;
    f32 Interpol = 0.0f;
    f32 StraightDist2 = 0.0f;
    f32 StraightDist1 = 0.0f;
    f32 TotalDist_Time = 0.0f;
    f32 OurTime = 0.0f;
    CVector CoorsOnLine1;
    CVector CoorsOnLine2;

    // Check if either distance is invalid (<= 0)
    if (DistToPoint1 <= 0.0f || DistToPoint2 <= 0.0f)
    {
        // Calculate straight-line distance between start and end coordinates
        CVector Delta = startCoors - endCoors;
        f32 StraightDist = Delta.Magnitude2D();

        // Calculate corrected distance
        f32 Interpol = 0.0f;
        f32 CorrectedDist = CalcCorrectedDist(StraightDist * Time, StraightDist, SpeedVariation, &Interpol);

        // Calculate coordinates along the curve
        resultCoor = startCoors + startDir * CorrectedDist;

        // Calculate speed vector
        f32 InvTime = 1.0f - Time;
        f32 TotalDist = StraightDist / (1.0f - SpeedVariation);
        f32 SpeedScale = TotalDist / (static_cast<f32>(TraverselTimeInMillis) * 0.001f);

        resultSpeed = (endDir * Time + startDir * InvTime) * SpeedScale;
        resultSpeed.z = 0.0f;  // Assuming 2D movement

        return;
    }

    // Determine the minimum valid distance (CMaths::Min twice like above?)
    f32 MinDist = CMaths::Min(DistToPoint1, DistToPoint2);
    MinDist = CMaths::Min(MinDist, 5.0f);

    // Calculate the remaining distances
    f32 BendDist1 = DistToPoint1 - MinDist;
    f32 BendDist2 = DistToPoint2 - MinDist;

    // Calculate total distance
    f32 TotalDist = MinDist + MinDist + BendDist1 + BendDist2;

    // Calculate current distance at the given time
    f32 CurrentDist = TotalDist * Time;

    // Calculate coordinates along the curve
    if (CurrentDist < BendDist1)
    {
        resultCoor = startCoors + startDir * CurrentDist;
    }
    else
    {
        f32 RemainingDist = CurrentDist - BendDist1;
        f32 Interpol = RemainingDist / (MinDist + MinDist);

        resultCoor = startCoors + startDir * BendDist1 + endDir * RemainingDist;
    }

    // Calculate speed vector
    f32 InvTime = 1.0f - Time;
    f32 SpeedScale = TotalDist / (static_cast<f32>(TraverselTimeInMillis) * 0.001f);

    resultSpeed = (endDir * Time + startDir * InvTime) * SpeedScale;
    resultSpeed.z = 0.0f;  // Assuming 2D movement
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
