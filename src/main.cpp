#include <windows.h>

#include <iostream>
#include <print>
#include <cassert>
#include <type_traits>

using f32 = float;
using i32 = int;
using u32 = unsigned int;

template <u32 addr, typename Ret = void, typename... Args>
inline Ret Call(Args... a)
{
    return reinterpret_cast<Ret(__cdecl*)(Args...)>(addr)(a...);
}

struct CVector
{
    f32 x, y, z;

    CVector() {}
    CVector(f32 _x, f32 _y, f32 _z) : x(_x), y(_y), z(_z) {}
};

class CCurves
{
public:
    CCurves() { TestCurves(); }

    static void TestCurves();

    /// Calculate the smallest distance needed for two mathematical lines to cross.
    ///
    /// \param LineBaseX The x-coordinate of the first line's base point.
    /// \param LineBaseY The y-coordinate of the first line's base point.
    /// \param LineDirX The x-component of the first line's direction vector.
    /// \param LineDirY The y-component of the first line's direction vector.
    /// \param OtherLineBaseX The x-coordinate of the other line's base point.
    /// \param OtherLineBaseY The y-coordinate of the other line's base point.
    /// \param OtherLineDirX The x-component of the other line's direction vector.
    /// \param OtherLineDirY The y-component of the other line's direction vector.
    ///
    /// \return The distance along the first line to the point where it crosses the other line.
    ///         Returns -1.0 if the lines are parallel and do not intersect.
    ///
    /// This function calculates the distance along the first line (defined by `lineStart` and `lineDir`)
    /// to the point where it intersects the other line (defined by `otherLineStart` and `otherLineDir`).
    /// If the lines are parallel (i.e., their direction vectors are linearly dependent), the function
    /// returns -1.0 to indicate that no intersection exists.
    static f32 DistForLineToCrossOtherLine(f32 LineBaseX, f32 LineBaseY, f32 LineDirX, f32 LineDirY, f32 OtherLineBaseX,
        f32 OtherLineBaseY, f32 OtherLineDirX, f32 OtherLineDirY);

    /// Calculates the speed variation when traversing a bend defined by the start and end coordinates and directions.
    /// \param startCoors The starting coordinates of the curve.
    /// \param endCoors The ending coordinates of the curve.
    /// \param StartDirX The x-component of the starting direction.
    /// \param StartDirY The y-component of the starting direction.
    /// \param EndDirX The x-component of the ending direction.
    /// \param EndDirY The y-component of the ending direction.
    /// \return The computed speed variation factor influenced by the bend in the curve.
    static f32 CalcSpeedVariationInBend(
        const CVector& startCoors, const CVector& endCoors, f32 StartDirX, f32 StartDirY, f32 EndDirX, f32 EndDirY);

    /// Computes the speed scaling factor for a curve defined by its start and end coordinates and directions.
    /// \param startCoors The starting coordinates of the curve.
    /// \param endCoors The ending coordinates of the curve.
    /// \param StartDirX The x-component of the starting direction.
    /// \param StartDirY The y-component of the starting direction.
    /// \param EndDirX The x-component of the ending direction.
    /// \param EndDirY The y-component of the ending direction.
    /// \return The calculated speed scaling factor for the curve.
    static f32 CalcSpeedScaleFactor(
        const CVector& startCoors, const CVector& endCoors, f32 StartDirX, f32 StartDirY, f32 EndDirX, f32 EndDirY);

    /// Calculates a point on the curve and the corresponding speed at a specified time.
    /// \param startCoors The starting coordinates of the curve.
    /// \param endCoors The ending coordinates of the curve.
    /// \param startDir The starting direction vector.
    /// \param endDir The ending direction vector.
    /// \param Time The time parameter (typically normalized between 0.0 and 1.0) used to interpolate along the curve.
    /// \param TraverselTimeInMillis The total traversal time in milliseconds for the curve.
    /// \param resultCoor The resulting interpolated coordinates on the curve.
    /// \param resultSpeed The resulting computed speed vector at the corresponding point on the curve.
    static void CalcCurvePoint(const CVector& startCoors, const CVector& endCoors, const CVector& startDir,
        const CVector& endDir, f32 Time, i32 TraverselTimeInMillis, CVector& resultCoor, CVector& resultSpeed);

    /// Computes the total length of a curve defined by its start and end coordinates and directions.
    /// \param startCoors The starting coordinates of the curve.
    /// \param endCoors The ending coordinates of the curve.
    /// \param startDir The starting direction vector.
    /// \param endDir The ending direction vector.
    /// \param pLength Pointer to a variable where the computed curve length will be stored.
    static void CalcCurveLength(const CVector& startCoors, const CVector& endCoors, const CVector& startDir,
        const CVector& endDir, f32* pLength);

    /// Calculates a corrected distance along the curve that accounts for speed variation.
    /// \param Current The current progression along the curve.
    /// \param Total The total progression or length of the curve.
    /// \param SpeedVariation The factor reflecting the speed variation along the curve.
    /// \param pInterPol Pointer to a variable that may receive additional interpolation data.
    /// \return The corrected distance along the curve considering the speed variation.
    static f32 CalcCorrectedDist(f32 Current, f32 Total, f32 SpeedVariation, f32* pInterPol);
} c; // init static

f32 CCurves::CalcCorrectedDist(f32 Current, f32 Total, f32 SpeedVariation,
                               f32 *pInterPol) {
  return Call<0x43C880, f32>(Current, Total, SpeedVariation, pInterPol);
}

void CCurves::CalcCurvePoint(const CVector &startCoors, const CVector &endCoors,
                             const CVector &startDir, const CVector &endDir,
                             f32 Time, i32 TraverselTimeInMillis,
                             CVector &resultCoor, CVector &resultSpeed) {
  Call<0x43C900>(&startCoors, &endCoors, &startDir, &endDir, Time,
                 TraverselTimeInMillis, &resultCoor, &resultSpeed);
}

f32 CCurves::CalcSpeedScaleFactor(const CVector &startCoors,
                                  const CVector &endCoors, f32 StartDirX,
                                  f32 StartDirY, f32 EndDirX, f32 EndDirY) {
  return Call<0x43C710, f32>(&startCoors, &endCoors, StartDirX, StartDirY,
                             EndDirX, EndDirY);
}

f32 CCurves::CalcSpeedVariationInBend(const CVector &startCoors,
                                      const CVector &endCoors, f32 StartDirX,
                                      f32 StartDirY, f32 EndDirX, f32 EndDirY) {
  return Call<0x43C660, f32>(&startCoors, &endCoors, StartDirX, StartDirY,
                             EndDirX, EndDirY);
}

f32 CCurves::DistForLineToCrossOtherLine(f32 LineBaseX, f32 LineBaseY,
                                         f32 LineDirX, f32 LineDirY,
                                         f32 OtherLineBaseX, f32 OtherLineBaseY,
                                         f32 OtherLineDirX, f32 OtherLineDirY) {
  return Call<0x43C610, f32>(LineBaseX, LineBaseY, LineDirX, LineDirY,
                             OtherLineBaseX, OtherLineBaseY, OtherLineDirX,
                             OtherLineDirY);
}

void cn_init_console()
{
    AllocConsole();

    // TODO(iFarbod):
    // SetConsoleCtrlHandler()

    SECURITY_ATTRIBUTES security_attributes = {sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};

    HANDLE win_handle = CreateFileW(L"CONOUT$", GENERIC_WRITE, FILE_SHARE_WRITE, &security_attributes, CREATE_ALWAYS,
        FILE_FLAG_NO_BUFFERING, nullptr);

    _wfreopen(L"CONOUT$", L"wb", stdout);

    SetStdHandle(STD_OUTPUT_HANDLE, win_handle);
}

void CCurves::TestCurves()
{
    cn_init_console();

    while (!IsDebuggerPresent())
    {
        Sleep(10);
    }

    // Test case 1: Lines intersect
    {
        f32 result =
            CCurves::DistForLineToCrossOtherLine(0.0f, 0.0f, 1.0f, 1.0f,  // Line 1: base (0,0), direction (1,1)
                1.0f, 0.0f, -1.0f, 1.0f                                   // Line 2: base (1,0), direction (-1,1)
            );

        std::println("Test 1: {}", result);
        assert(result == 0.5f);                                           // Expected distance to crossing
    }

    // Test case 2: Lines are parallel (no intersection)
    {
        f32 result =
            CCurves::DistForLineToCrossOtherLine(0.0f, 0.0f, 1.0f, 1.0f,  // Line 1: base (0,0), direction (1,1)
                1.0f, 1.0f, 1.0f, 1.0f                                    // Line 2: base (1,1), direction (1,1)
            );

        std::println("Test 2: {}", result);
        assert(result == -1.0f);                                          // Expected -1 for parallel lines
    }

    // Test case 3: Lines intersect at a point
    {
        f32 result =
            CCurves::DistForLineToCrossOtherLine(0.0f, 0.0f, 2.0f, 2.0f,  // Line 1: base (0,0), direction (2,2)
                0.0f, 4.0f, 2.0f, -2.0f                                   // Line 2: base (0,4), direction (2,-2)
            );

        std::println("Test 3: {}", result);
        assert(result == 1.0f);                                           // Expected distance to crossing
    }

    // Test case 4: Lines are coincident (infinite intersections)
    {
        f32 result =
            CCurves::DistForLineToCrossOtherLine(0.0f, 0.0f, 1.0f, 1.0f,  // Line 1: base (0,0), direction (1,1)
                1.0f, 1.0f, 2.0f, 2.0f                                    // Line 2: base (1,1), direction (2,2)
            );

        std::println("Test 4: {}", result);
        assert(result == -1.0f);                                          // Expected -1 for coincident lines
    }

    // Test Case 5: CalcSpeedVariationInBend - Simple Bend
    {
        CVector startCoors(0.0f, 0.0f, 0.0f);
        CVector endCoors(1.0f, 1.0f, 0.0f);
        f32 speedVariation = CalcSpeedVariationInBend(startCoors, endCoors, 1.0f, 0.0f, 0.0f, 1.0f);

        std::println("Test 5: speedVariation {}", speedVariation);
        assert(speedVariation >= 0.0f && speedVariation <= 1.0f && "Test Case 5 Failed: Speed variation out of range.");
    }

#if 0
    // Test Case 6: CalcSpeedScaleFactor - Simple Bend
    {
        CVector startCoors(0.0f, 0.0f, 0.0f);
        CVector endCoors(1.0f, 1.0f, 0.0f);
        f32 speedScaleFactor = CalcSpeedScaleFactor(startCoors, endCoors, 1.0f, 0.0f, 0.0f, 1.0f);

        std::println("Test 6: speedScaleFactor {}", speedScaleFactor);
        assert(speedScaleFactor > 0.0f && "Test Case 6 Failed: Speed scale factor should be positive.");
    }
#endif

    // Test Case 7: CalcSpeedScaleFactor - Simple Bend
    {
        CVector startCoors(0.0f, 0.0f, 0.0f);
        CVector endCoors(1.0f, 1.0f, 0.0f);
        f32 speedScaleFactor = CalcSpeedScaleFactor(startCoors, endCoors, 1.0f, 0.0f, 0.0f, 1.0f);

        std::println("Test 7: speedScaleFactor {}", speedScaleFactor);
        assert(fabs(speedScaleFactor - 2.0f) < 0.0001f && "Test Case 6 Failed: Incorrect speed scale factor.");
    }

    // Test Case 8: CalcSpeedScaleFactor - Straight Line
    {
        CVector startCoors(0.0f, 0.0f, 0.0f);
        CVector endCoors(1.0f, 0.0f, 0.0f);
        f32 speedScaleFactor = CalcSpeedScaleFactor(startCoors, endCoors, 1.0f, 0.0f, 1.0f, 0.0f);

        std::println("Test 8: speedScaleFactor {}", speedScaleFactor);
        assert(fabs(speedScaleFactor - 1.0f) < 0.0001f && "Test Case 7 Failed: Incorrect speed scale factor.");
    }

    // Test Case 9: CalcSpeedScaleFactor - Sharp Bend
    {
        CVector startCoors(0.0f, 0.0f, 0.0f);
        CVector endCoors(1.0f, 0.0f, 0.0f);
        f32 speedScaleFactor = CalcSpeedScaleFactor(startCoors, endCoors, 1.0f, 0.0f, 0.0f, 1.0f);

        std::println("Test 9: speedScaleFactor {}", speedScaleFactor);
        assert(speedScaleFactor > 1.0f && "Test Case 8 Failed: Speed scale factor should be greater than 1.0.");
    }

    // Test Case 10: CalcSpeedScaleFactor - Parallel Lines
    {
        CVector startCoors(0.0f, 0.0f, 0.0f);
        CVector endCoors(1.0f, 0.0f, 0.0f);
        f32 speedScaleFactor = CalcSpeedScaleFactor(startCoors, endCoors, 1.0f, 0.0f, 1.0f, 0.0f);

        std::println("Test 10: speedScaleFactor {}", speedScaleFactor);
        assert(fabs(speedScaleFactor - 1.0f) < 0.0001f && "Test Case 9 Failed: Incorrect speed scale factor.");
    }

    // Test Case 11: CalcSpeedScaleFactor - Coincident Lines
    {
        CVector startCoors(0.0f, 0.0f, 0.0f);
        CVector endCoors(1.0f, 1.0f, 0.0f);
        f32 speedScaleFactor = CalcSpeedScaleFactor(startCoors, endCoors, 1.0f, 1.0f, 1.0f, 1.0f);

        std::println("Test 11: speedScaleFactor {}", speedScaleFactor);
        assert(fabs(speedScaleFactor - 1.4142135f) < 0.0001f && "Test Case 10 Failed: Incorrect speed scale factor.");
    }

    // Test Case 12: CalcCurvePoint - Straight Line
    {
        CVector startCoors(0.0f, 0.0f, 0.0f);
        CVector endCoors(1.0f, 0.0f, 0.0f);
        CVector startDir(1.0f, 0.0f, 0.0f);
        CVector endDir(1.0f, 0.0f, 0.0f);
        CVector resultCoor, resultSpeed;
        CalcCurvePoint(startCoors, endCoors, startDir, endDir, 0.5f, 1000, resultCoor, resultSpeed);

        std::println("Test 12: x{} y{} z{}", resultCoor.x, resultCoor.y, resultCoor.z);
        assert(resultCoor.x == 0.5f && resultCoor.y == 0.0f && resultCoor.z == 0.0f &&
               "Test Case 12 Failed: Incorrect curve point.");
    }

    // Test Case 13: CalcCorrectedDist - Simple Case
    {
        f32 interpol;
        f32 correctedDist = CalcCorrectedDist(0.5f, 1.0f, 0.5f, &interpol);

        std::println("Test 13: correctedDist {} interpol {}", correctedDist, interpol);
        assert(
            correctedDist >= 0.0f && correctedDist <= 1.0f && "Test Case 13 Failed: Corrected distance out of range.");
    }

    // Test 1: 0.5
    // Test 2: -1
    // Test 3: 1
    // Test 4: -1
    // Test 5: speedVariation 0.3333333

    // Test 6: speedScaleFactor 2
    // Test 7: speedScaleFactor 2
    // Test 8: speedScaleFactor 1
    // Test 9: speedScaleFactor 1.5
    // Test 10: speedScaleFactor 1
    // Test 11: speedScaleFactor 1.4142135

    // Test 12: x0.5 y0 z0
    // Test 13: correctedDist 0.25 interpol 0.5

    __debugbreak();
    Sleep(5000);
}
