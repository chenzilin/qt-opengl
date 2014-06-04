#include "tray.h"

// Line betwen two points OR point and a direction
TRay::TRay(const TVector &point1, const TVector &point2)
: _P(point1)
{
    _V = (point2.isUnit() ? point2 : TVector::unit(point2-point1));
}

// Distance between a ray and a point
double TRay::dist(const TVector &point) const
{
    if (isValid() && point.isValid())
    {
        TVector tv, point2;
        double lambda = TVector::dot(_V, point-_P);
        TVector::add(_P, TVector::multiply(_V, lambda, tv), point2);
        return point.dist(point2);
    }
    return 0.0;
}
