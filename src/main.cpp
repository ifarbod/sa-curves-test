#include <windows.h>

#include <cassert>
#include <iostream>
#include <print>
#include <type_traits>

#include "curves.hpp"

#define epsilon 0.0000001f
#define FLOAT_EQUAL(a, b) (fabs((a) - (b)) < (epsilon))

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
    auto DistForLineToCrossOtherLine_test = []
    {
        // Test case 1: Lines intersect
        {
            f32 result = DistForLineToCrossOtherLine(0.0f, 0.0f, 1.0f, 1.0f,  // Line 1: base (0,0), direction (1,1)
                1.0f, 0.0f, -1.0f, 1.0f                                       // Line 2: base (1,0), direction (-1,1)
            );

            // 0.5
            assert(FLOAT_EQUAL(result, 0.5f));  // Expected distance to crossing
        }

        // Test case 2: Lines are parallel (no intersection)
        {
            f32 result = DistForLineToCrossOtherLine(0.0f, 0.0f, 1.0f, 1.0f,  // Line 1: base (0,0), direction (1,1)
                1.0f, 1.0f, 1.0f, 1.0f                                        // Line 2: base (1,1), direction (1,1)
            );

            // -1
            assert(FLOAT_EQUAL(result, -1.0f));  // Expected -1 for parallel lines
        }

        // Test case 3: Lines intersect at a point
        {
            f32 result = DistForLineToCrossOtherLine(0.0f, 0.0f, 2.0f, 2.0f,  // Line 1: base (0,0), direction (2,2)
                0.0f, 4.0f, 2.0f, -2.0f                                       // Line 2: base (0,4), direction (2,-2)
            );

            // 1.0
            assert(FLOAT_EQUAL(result, 1.0f));  // Expected distance to crossing
        }

        // Test case 4: Lines are coincident (infinite intersections)
        {
            f32 result = DistForLineToCrossOtherLine(0.0f, 0.0f, 1.0f, 1.0f,  // Line 1: base (0,0), direction (1,1)
                1.0f, 1.0f, 2.0f, 2.0f                                        // Line 2: base (1,1), direction (2,2)
            );

            // -1
            assert(FLOAT_EQUAL(result, -1.0f));  // Expected -1 for coincident lines
        }

        // Test case 5: Lines with large numbers
        {
            f32 result = DistForLineToCrossOtherLine(
                2500.5f, 1500.0f, 3.5f, 2.5f,  // Line 1: base (2500.5,1500), direction (3.5,2.5)
                3000.0f, 2000.0f, -4.0f, 3.0f  // Line 2: base (3000,2000), direction (-4,3)
            );

            // Should still give a reasonable intersection distance
            assert(FLOAT_EQUAL(result, 170.658539f));  // Expected distance to crossing
        }
    };

    auto CalcSpeedVariationInBend_test = []
    {
        // Test Case 1: Dot product <= 0 (opposite directions)
        {
            CVector startCoors(0.0f, 0.0f, 0.0f);
            CVector endCoors(1.0f, 0.0f, 0.0f);
            f32 speedVariation =
                CalcSpeedVariationInBend(startCoors, endCoors, 1.0f, 0.0f, -1.0f, 0.0f);  // Opposite directions
            assert(FLOAT_EQUAL(speedVariation, 1.0f / 3.0f) && "Test Case 1 Failed: Expected 0.33333.");
        }

        // Test Case 2: Dot product <= 0 (perpendicular directions)
        {
            CVector startCoors(0.0f, 0.0f, 0.0f);
            CVector endCoors(1.0f, 1.0f, 0.0f);
            f32 speedVariation =
                CalcSpeedVariationInBend(startCoors, endCoors, 1.0f, 0.0f, 0.0f, 1.0f);  // Perpendicular directions
            assert(FLOAT_EQUAL(speedVariation, 1.0f / 3.0f) && "Test Case 2 Failed: Expected 0.33333.");
        }

        // Test Case 3: Dot product <= 0.7 (small angle)
        {
            CVector startCoors(0.0f, 0.0f, 0.0f);
            CVector endCoors(1.0f, 0.0f, 0.0f);
            f32 speedVariation = CalcSpeedVariationInBend(startCoors, endCoors, 1.0f, 0.0f, 0.9f, 0.1f);  // Small angle
            assert(FLOAT_EQUAL(speedVariation, 0.145296633f) &&
                   "Test Case 3 Failed: Expected value between 0 and 0.33333.");
        }

        // Test Case 4: Dot product > 0.7 (larger angle)
        {
            CVector startCoors(0.0f, 0.0f, 0.0f);
            CVector endCoors(1.0f, 1.0f, 0.0f);
            f32 speedVariation =
                CalcSpeedVariationInBend(startCoors, endCoors, 1.0f, 0.0f, 0.8f, 0.2f);  // Larger angle
            assert(FLOAT_EQUAL(speedVariation, 0.235702246f) &&
                   "Test Case 4 Failed: Expected value between 0 and 0.33333.");
        }

        // Test Case 5: Dot product = 0.7 (boundary case)
        {
            CVector startCoors(0.0f, 0.0f, 0.0f);
            CVector endCoors(1.0f, 0.0f, 0.0f);
            f32 speedVariation =
                CalcSpeedVariationInBend(startCoors, endCoors, 1.0f, 0.0f, 0.7f, 0.7141428f);  // Dot product = 0.7
            assert(FLOAT_EQUAL(speedVariation, 0.0f) && "Test Case 5 Failed: Expected 0.0.");
        }

        // Test Case 6: Dot product > 0.7 (sharp bend)
        {
            CVector startCoors(0.0f, 0.0f, 0.0f);
            CVector endCoors(1.0f, 0.0f, 0.0f);
            f32 speedVariation = CalcSpeedVariationInBend(startCoors, endCoors, 1.0f, 0.0f, 0.6f, 0.8f);  // Sharp bend
            assert(
                FLOAT_EQUAL(speedVariation, 0.047619f) && "Test Case 6 Failed: Expected value between 0 and 0.33333.");
        }

        // Test Case 7: Dot product = 1 (same direction)
        {
            CVector startCoors(0.0f, 0.0f, 0.0f);
            CVector endCoors(1.0f, 0.0f, 0.0f);
            f32 speedVariation =
                CalcSpeedVariationInBend(startCoors, endCoors, 1.0f, 0.0f, 1.0f, 0.0f);  // Same direction
            assert(speedVariation == 0.0f && "Test Case 7 Failed: Expected 0.0.");
        }

        // Test Case 8: Dot product > 0.7 (perpendicular distance calculation)
        {
            CVector startCoors(0.0f, 0.0f, 0.0f);
            CVector endCoors(1.0f, 1.0f, 0.0f);
            f32 speedVariation =
                CalcSpeedVariationInBend(startCoors, endCoors, 1.0f, 0.0f, 0.0f, 1.0f);  // Perpendicular directions
            assert(speedVariation > 0.0f && speedVariation <= (1.0f / 3.0f) &&
                   "Test Case 8 Failed: Expected value between 0 and 0.33333.");
        }

        // Test Case 9: Dot product > 0.7 (large perpendicular distance)
        {
            CVector startCoors(0.0f, 0.0f, 0.0f);
            CVector endCoors(10.0f, 0.0f, 0.0f);
            f32 speedVariation =
                CalcSpeedVariationInBend(startCoors, endCoors, 1.0f, 0.0f, 0.0f, 1.0f);  // Large perpendicular distance
            assert(speedVariation > 0.0f && speedVariation <= (1.0f / 3.0f) &&
                   "Test Case 9 Failed: Expected value between 0 and 0.33333.");
        }

        // Test Case 10: Dot product > 0.7 (small perpendicular distance)
        {
            CVector startCoors(0.0f, 0.0f, 0.0f);
            CVector endCoors(1.0f, 0.1f, 0.0f);
            f32 speedVariation =
                CalcSpeedVariationInBend(startCoors, endCoors, 1.0f, 0.0f, 0.0f, 1.0f);  // Small perpendicular distance
            assert(speedVariation > 0.0f && speedVariation <= (1.0f / 3.0f) &&
                   "Test Case 10 Failed: Expected value between 0 and 0.33333.");
        }
    };

    auto CalcSpeedScaleFactor_test = []
    {
        // Test Case 1: CalcSpeedScaleFactor - Simple Bend
        {
            CVector startCoors(0.0f, 0.0f, 0.0f);
            CVector endCoors(1.0f, 1.0f, 0.0f);
            f32 speedScaleFactor = CalcSpeedScaleFactor(startCoors, endCoors, 1.0f, 0.0f, 0.0f, 1.0f);

            // 2.0
            assert(FLOAT_EQUAL(speedScaleFactor, 2.0f) && "Test Case Failed: Incorrect speed scale factor.");
        }

        // Test Case 2: CalcSpeedScaleFactor - Straight Line
        {
            CVector startCoors(0.0f, 0.0f, 0.0f);
            CVector endCoors(1.0f, 0.0f, 0.0f);
            f32 speedScaleFactor = CalcSpeedScaleFactor(startCoors, endCoors, 1.0f, 0.0f, 1.0f, 0.0f);

            // 1.0
            assert(FLOAT_EQUAL(speedScaleFactor, 1.0f) && "Test Case Failed: Incorrect speed scale factor.");
        }

        // Test Case 3: CalcSpeedScaleFactor - Sharp Bend
        {
            CVector startCoors(0.0f, 0.0f, 0.0f);
            CVector endCoors(1.0f, 0.0f, 0.0f);
            f32 speedScaleFactor = CalcSpeedScaleFactor(startCoors, endCoors, 1.0f, 0.0f, 0.0f, 1.0f);

            // 1.5
            assert(FLOAT_EQUAL(speedScaleFactor, 1.5f) &&
                   "Test Case Failed: Speed scale factor should be greater than 1.0.");
        }

        // Test Case 4: CalcSpeedScaleFactor - Parallel Lines
        {
            CVector startCoors(0.0f, 0.0f, 0.0f);
            CVector endCoors(1.0f, 0.0f, 0.0f);
            f32 speedScaleFactor = CalcSpeedScaleFactor(startCoors, endCoors, 1.0f, 0.0f, 1.0f, 0.0f);

            // 1.0
            assert(FLOAT_EQUAL(speedScaleFactor, 1.0f) && "Test Case Failed: Incorrect speed scale factor.");
        }

        // Test Case 5: CalcSpeedScaleFactor - Coincident Lines
        {
            CVector startCoors(0.0f, 0.0f, 0.0f);
            CVector endCoors(1.0f, 1.0f, 0.0f);
            f32 speedScaleFactor = CalcSpeedScaleFactor(startCoors, endCoors, 1.0f, 1.0f, 1.0f, 1.0f);

            // ~1.41
            assert(FLOAT_EQUAL(speedScaleFactor, 1.4142135f) && "Test Case Failed: Incorrect speed scale factor.");
        }

        // Test Case 6: CalcSpeedScaleFactor - Large Values
        {
            CVector startCoors(2500.0f, 1500.0f, 0.0f);
            CVector endCoors(3500.0f, 2000.0f, 0.0f);
            f32 speedScaleFactor = CalcSpeedScaleFactor(startCoors, endCoors, 2.0f, 1.0f, 3.0f, 2.0f);

            // Should be a reasonable scale factor despite large coordinates
            assert(FLOAT_EQUAL(speedScaleFactor, 1118.03394f) &&
                   "Test Case Failed: Speed scale factor out of reasonable range for large values.");
        }
    };

    auto CalcCurvePoint_test = []
    {
        // Test Case 1: CalcCurvePoint - Straight Line
        {
            CVector startCoors(0.0f, 0.0f, 0.0f);
            CVector endCoors(1.0f, 0.0f, 0.0f);
            CVector startDir(1.0f, 0.0f, 0.0f);
            CVector endDir(1.0f, 0.0f, 0.0f);
            CVector resultCoor;
            CVector resultSpeed;
            CalcCurvePoint(startCoors, endCoors, startDir, endDir, 0.5f, 1000, resultCoor, resultSpeed);

            assert(FLOAT_EQUAL(resultCoor.x, 0.5f) && FLOAT_EQUAL(resultCoor.y, 0.0f) &&
                   FLOAT_EQUAL(resultCoor.z, 0.0f) && "Test Case 1 Failed: Incorrect curve point.");
        }

        // Test Case 2: CalcCurvePoint - Curve with Bend
        {
            CVector startCoors(0.0f, 0.0f, 0.0f);
            CVector endCoors(1.0f, 1.0f, 0.0f);
            CVector startDir(1.0f, 0.0f, 0.0f);
            CVector endDir(0.0f, 1.0f, 0.0f);
            CVector resultCoor;
            CVector resultSpeed;
            CalcCurvePoint(startCoors, endCoors, startDir, endDir, 0.5f, 1000, resultCoor, resultSpeed);

            // Expected result depends on the interpolation logic
            assert(resultCoor.x > 0.0f && resultCoor.x < 1.0f && resultCoor.y > 0.0f && resultCoor.y < 1.0f &&
                   FLOAT_EQUAL(resultCoor.z, 0.0f) && "Test Case 2 Failed: Incorrect curve point.");
        }

        // Test Case 3: CalcCurvePoint - Large Values
        {
            CVector startCoors(0.0f, 0.0f, 0.0f);
            CVector endCoors(1000.0f, 1000.0f, 0.0f);
            CVector startDir(1.0f, 0.0f, 0.0f);
            CVector endDir(0.0f, 1.0f, 0.0f);
            CVector resultCoor;
            CVector resultSpeed;
            CalcCurvePoint(startCoors, endCoors, startDir, endDir, 0.5f, 1000, resultCoor, resultSpeed);

            // Expected result depends on the interpolation logic
            assert(resultCoor.x > 0.0f && resultCoor.x < 1000.0f && resultCoor.y > 0.0f && resultCoor.y < 1000.0f &&
                   FLOAT_EQUAL(resultCoor.z, 0.0f) && "Test Case 3 Failed: Incorrect curve point.");
        }

        // Test Case 4: CalcCurvePoint - Time = 0.0 (Start Point)
        {
            CVector startCoors(0.0f, 0.0f, 0.0f);
            CVector endCoors(1.0f, 1.0f, 0.0f);
            CVector startDir(1.0f, 0.0f, 0.0f);
            CVector endDir(0.0f, 1.0f, 0.0f);
            CVector resultCoor;
            CVector resultSpeed;
            CalcCurvePoint(startCoors, endCoors, startDir, endDir, 0.0f, 1000, resultCoor, resultSpeed);

            assert(FLOAT_EQUAL(resultCoor.x, 0.0f) && FLOAT_EQUAL(resultCoor.y, 0.0f) &&
                   FLOAT_EQUAL(resultCoor.z, 0.0f) && "Test Case 4 Failed: Incorrect curve point.");
        }

        // Test Case 5: CalcCurvePoint - Time = 1.0 (End Point)
        {
            CVector startCoors(0.0f, 0.0f, 0.0f);
            CVector endCoors(1.0f, 1.0f, 0.0f);
            CVector startDir(1.0f, 0.0f, 0.0f);
            CVector endDir(0.0f, 1.0f, 0.0f);
            CVector resultCoor;
            CVector resultSpeed;
            CalcCurvePoint(startCoors, endCoors, startDir, endDir, 1.0f, 1000, resultCoor, resultSpeed);

            assert(FLOAT_EQUAL(resultCoor.x, 1.0f) && FLOAT_EQUAL(resultCoor.y, 1.0f) &&
                   FLOAT_EQUAL(resultCoor.z, 0.0f) && "Test Case 5 Failed: Incorrect curve point.");
        }

        // Test Case 6: CalcCurvePoint - Z-Axis Movement
        {
            CVector startCoors(0.0f, 0.0f, 0.0f);
            CVector endCoors(0.0f, 0.0f, 1.0f);
            CVector startDir(0.0f, 0.0f, 1.0f);
            CVector endDir(0.0f, 0.0f, 1.0f);
            CVector resultCoor;
            CVector resultSpeed;
            CalcCurvePoint(startCoors, endCoors, startDir, endDir, 0.5f, 1000, resultCoor, resultSpeed);

            assert(FLOAT_EQUAL(resultCoor.x, 0.0f) && FLOAT_EQUAL(resultCoor.y, 0.0f) &&
                   FLOAT_EQUAL(resultCoor.z, 0.5f) && "Test Case 6 Failed: Incorrect curve point.");
        }

        // Test Case 7: CalcCurvePoint - Sharp Bend
        {
            CVector startCoors(0.0f, 0.0f, 0.0f);
            CVector endCoors(1.0f, 0.0f, 0.0f);
            CVector startDir(1.0f, 0.0f, 0.0f);
            CVector endDir(-1.0f, 0.0f, 0.0f);
            CVector resultCoor;
            CVector resultSpeed;
            CalcCurvePoint(startCoors, endCoors, startDir, endDir, 0.5f, 1000, resultCoor, resultSpeed);

            // Expected result depends on the interpolation logic
            assert(FLOAT_EQUAL(resultCoor.x, 1.0f) && FLOAT_EQUAL(resultCoor.y, 0.0f) &&
                   FLOAT_EQUAL(resultCoor.z, 0.0f) && "Test Case 7 Failed: Incorrect curve point.");
        }
    };

    auto CalcCorrectedDist_test = []
    {
        // Test Case 1: CalcCorrectedDist - Simple Case
        {
            // interpol please don't arrest us, we have done nothing!
            f32 interpol = 0.0f;
            f32 correctedDist = CalcCorrectedDist(0.5f, 1.0f, 0.5f, &interpol);

            assert(FLOAT_EQUAL(correctedDist, 0.25f) && FLOAT_EQUAL(interpol, 0.5f) &&
                   "Test Case 1 Failed: Corrected distance out of range.");
        }
        // Test Case 2: CalcCorrectedDist - Start of Curve (Time = 0.0)
        {
            f32 interpol = 0.0f;
            f32 correctedDist = CalcCorrectedDist(0.0f, 1.0f, 0.5f, &interpol);

            assert(FLOAT_EQUAL(correctedDist, 0.0f) && FLOAT_EQUAL(interpol, 0.0f) &&
                   "Test Case 2 Failed: Corrected distance should be 0.0 at the start.");
        }

        // Test Case 3: CalcCorrectedDist - End of Curve (Time = 1.0)
        {
            f32 interpol = 0.0f;
            f32 correctedDist = CalcCorrectedDist(1.0f, 1.0f, 0.5f, &interpol);

            assert(FLOAT_EQUAL(correctedDist, 0.5f) && FLOAT_EQUAL(interpol, 1.0f) &&
                   "Test Case 3 Failed: Corrected distance should be 1.0 at the end.");
        }

        // Test Case 4: CalcCorrectedDist - No Speed Variation (SpeedVariation = 0.0)
        {
            f32 interpol = 0.0f;
            f32 correctedDist = CalcCorrectedDist(0.5f, 1.0f, 0.0f, &interpol);

            assert(FLOAT_EQUAL(correctedDist, 0.5f) && FLOAT_EQUAL(interpol, 0.5f) &&
                   "Test Case 4 Failed: Corrected distance should match input when SpeedVariation is 0.0.");
        }

        // Test Case 5: CalcCorrectedDist - Maximum Speed Variation (SpeedVariation = 1.0)
        {
            f32 interpol = 0.0f;
            f32 correctedDist = CalcCorrectedDist(0.5f, 1.0f, 1.0f, &interpol);

            assert(FLOAT_EQUAL(correctedDist, 0.0f) && FLOAT_EQUAL(interpol, 0.5f) &&
                   "Test Case 5 Failed: Corrected distance should be 0.0 when SpeedVariation is 1.0.");
        }

        // Test Case 6: CalcCorrectedDist - Large Total Distance
        {
            f32 interpol = 0.0f;
            f32 correctedDist = CalcCorrectedDist(500.0f, 1000.0f, 0.5f, &interpol);

            assert(FLOAT_EQUAL(correctedDist, 250.0f) && FLOAT_EQUAL(interpol, 0.5f) &&
                   "Test Case 6 Failed: Corrected distance should scale with large Total distance.");
        }

        // Test Case 7: CalcCorrectedDist - Small Total Distance
        {
            f32 interpol = 0.0f;
            f32 correctedDist = CalcCorrectedDist(0.1f, 0.2f, 0.5f, &interpol);

            assert(FLOAT_EQUAL(correctedDist, 0.05f) && FLOAT_EQUAL(interpol, 0.5f) &&
                   "Test Case 7 Failed: Corrected distance should scale with small Total distance.");
        }

        // Test Case 8: CalcCorrectedDist - Negative Current Distance
        {
            f32 interpol = 0.0f;
            f32 correctedDist = CalcCorrectedDist(-0.5f, 1.0f, 0.5f, &interpol);

            assert(FLOAT_EQUAL(correctedDist, -0.25f) && FLOAT_EQUAL(interpol, 0.5f) &&
                   "Test Case 8 Failed: Corrected distance should handle negative Current distance.");
        }

        // Test Case 9: CalcCorrectedDist - Negative Total Distance
        {
            f32 interpol = 0.0f;
            f32 correctedDist = CalcCorrectedDist(0.5f, -1.0f, 0.5f, &interpol);

            assert(FLOAT_EQUAL(correctedDist, 0.0f) && FLOAT_EQUAL(interpol, 0.5f) &&
                   "Test Case 9 Failed: Corrected distance should handle negative Total distance.");
        }

        // Test Case 10: CalcCorrectedDist - SpeedVariation > 1.0
        {
            f32 interpol = 0.0f;
            f32 correctedDist = CalcCorrectedDist(0.5f, 1.0f, 1.5f, &interpol);

            assert(FLOAT_EQUAL(correctedDist, -0.25f) && FLOAT_EQUAL(interpol, 0.5f) &&
                   "Test Case 10 Failed: Corrected distance should handle SpeedVariation > 1.0.");
        }
    };

    DistForLineToCrossOtherLine_test();
    CalcSpeedVariationInBend_test();
    CalcSpeedScaleFactor_test();
    CalcCurvePoint_test();
    CalcCorrectedDist_test();

    __debugbreak();
    Sleep(5000);
}

// No need for DllMain
struct TestRunner
{
    TestRunner()
    {
        cn_init_console();

        while (!IsDebuggerPresent())
        {
            Sleep(10);
        }

        CCurves::TestCurves();
    }
} runner;
