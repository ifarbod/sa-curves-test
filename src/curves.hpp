// from types.hpp
using f32 = float;
using i32 = int;
using u32 = unsigned int;

template <u32 addr, typename Ret = void, typename... Args>
inline Ret Call(Args... a)
{
    return reinterpret_cast<Ret(__cdecl*)(Args...)>(addr)(a...);
}

// minimal vector class, no need to bring the whole thing over here
struct CVector
{
    f32 x, y, z;

    CVector() {}
    CVector(f32 _x, f32 _y, f32 _z) : x(_x), y(_y), z(_z) {}
};


class CCurves
{
public:
    /// Runs the unit tests
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
    /// This function determines whether and where two infinite lines, defined by their base points and
    /// direction vectors, will intersect. It computes the scaling factor for the first line's direction vector such
    /// that, when applied, the corresponding point lies on both lines. If the direction vectors are linearly dependent
    /// (i.e., the lines are parallel), the function concludes that an intersection does not occur and returns -1.0. The
    /// returned distance represents the smallest distance along the first line necessary to reach the crossing point
    /// with the second line, if such a crossing exists.
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
    ///
    /// This function computes how much the speed should vary while moving through a curved path
    /// based on the bend's geometry. It analyzes the curve defined by start/end points and their
    /// corresponding direction vectors to determine an appropriate speed variation factor.
    ///
    /// The variation factor helps adjust vehicle speed based on curve sharpness - sharper curves
    /// generally require more speed reduction compared to gentle curves. This creates more
    /// realistic and smoother motion through curved paths.
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
    ///
    /// This function calculates a scaling factor that adjusts the speed of an object moving along a curve.
    /// The scaling factor is influenced by the geometry of the curve, specifically the angle between the
    /// start and end direction vectors. A sharper bend (larger angle) results in a higher scaling factor,
    /// indicating that the object should slow down more to navigate the curve smoothly. Conversely, a gentle
    /// bend (smaller angle) results in a lower scaling factor, allowing the object to maintain a higher speed.
    ///
    /// The function first calculates the speed variation using `CalcSpeedVariationInBend` and then uses this
    /// variation to compute the final scaling factor. If the lines defined by the start and end points do not
    /// intersect, the function falls back to a straight-line distance calculation.
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
    ///
    /// This function computes the position and speed of an object moving along a curve at a specific time.
    /// The curve is defined by its start and end coordinates and direction vectors. The `Time` parameter
    /// determines how far along the curve the object has progressed, with 0.0 representing the start and 1.0
    /// representing the end.
    ///
    /// The function uses the `CalcCorrectedDist` function to account for speed variations along the curve,
    /// ensuring that the object's speed is adjusted appropriately based on the curve's geometry. The resulting
    /// position and speed are stored in `resultCoor` and `resultSpeed`, respectively.
    static void CalcCurvePoint(const CVector& startCoors, const CVector& endCoors, const CVector& startDir,
        const CVector& endDir, f32 Time, i32 TraverselTimeInMillis, CVector& resultCoor, CVector& resultSpeed);

    /// Computes the total length of a curve defined by its start and end coordinates and directions.
    /// \param startCoors The starting coordinates of the curve.
    /// \param endCoors The ending coordinates of the curve.
    /// \param startDir The starting direction vector.
    /// \param endDir The ending direction vector.
    /// \param pLength Pointer to a variable where the computed curve length will be stored.
    ///
    /// This function calculates the total length of a curve defined by its start and end points and their
    /// corresponding direction vectors. The curve length is computed by integrating the distance along the
    /// curve, taking into account the geometry of the bend.
    ///
    /// The function uses the `DistForLineToCrossOtherLine` function to determine the intersection points
    /// of the lines defined by the start and end points. If the lines do not intersect, the function falls
    /// back to calculating the straight-line distance between the start and end points.
    static void CalcCurveLength(const CVector& startCoors, const CVector& endCoors, const CVector& startDir,
        const CVector& endDir, f32* pLength);

    /// Calculates a corrected distance along the curve that accounts for speed variation.
    /// \param Current The current progression along the curve.
    /// \param Total The total progression or length of the curve.
    /// \param SpeedVariation The factor reflecting the speed variation along the curve.
    /// \param pInterPol Pointer to a variable that may receive additional interpolation data.
    /// \return The corrected distance along the curve considering the speed variation.
    ///
    /// This function computes a corrected distance along a curve that accounts for variations in speed
    /// caused by the curve's geometry. The `Current` parameter represents the object's current position
    /// along the curve, while the `Total` parameter represents the total length of the curve.
    ///
    /// The `SpeedVariation` factor is used to adjust the distance calculation, ensuring that the object's
    /// speed is appropriately scaled based on the sharpness of the curve. The function also provides an
    /// interpolation value (`pInterPol`) that can be used for further calculations or visual effects.
    static f32 CalcCorrectedDist(f32 Current, f32 Total, f32 SpeedVariation, f32* pInterPol);
};
