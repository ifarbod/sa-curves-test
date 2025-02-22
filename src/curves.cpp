#include "curves.hpp"

f32 CCurves::CalcCorrectedDist(f32 Current, f32 Total, f32 SpeedVariation, f32* pInterPol)
{
    return Call<0x43C880, f32>(Current, Total, SpeedVariation, pInterPol);
}

void CCurves::CalcCurvePoint(const CVector& startCoors, const CVector& endCoors, const CVector& startDir,
    const CVector& endDir, f32 Time, i32 TraverselTimeInMillis, CVector& resultCoor, CVector& resultSpeed)
{
    Call<0x43C900>(&startCoors, &endCoors, &startDir, &endDir, Time, TraverselTimeInMillis, &resultCoor, &resultSpeed);
}

f32 CCurves::CalcSpeedScaleFactor(
    const CVector& startCoors, const CVector& endCoors, f32 StartDirX, f32 StartDirY, f32 EndDirX, f32 EndDirY)
{
    return Call<0x43C710, f32>(&startCoors, &endCoors, StartDirX, StartDirY, EndDirX, EndDirY);
}

f32 CCurves::CalcSpeedVariationInBend(
    const CVector& startCoors, const CVector& endCoors, f32 StartDirX, f32 StartDirY, f32 EndDirX, f32 EndDirY)
{
    return Call<0x43C660, f32>(&startCoors, &endCoors, StartDirX, StartDirY, EndDirX, EndDirY);
}

f32 CCurves::DistForLineToCrossOtherLine(f32 LineBaseX, f32 LineBaseY, f32 LineDirX, f32 LineDirY, f32 OtherLineBaseX,
    f32 OtherLineBaseY, f32 OtherLineDirX, f32 OtherLineDirY)
{
    return Call<0x43C610, f32>(
        LineBaseX, LineBaseY, LineDirX, LineDirY, OtherLineBaseX, OtherLineBaseY, OtherLineDirX, OtherLineDirY);
}
