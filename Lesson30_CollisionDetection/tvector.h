#ifndef TVECTOR_H
#define TVECTOR_H

#include <math.h>
#include "mathex.h"

class TVector
{
public:
    enum TStatus { INVALID, DEFAULT, UNIT };
    // Constructors
    TVector() : _x(0.0), _y(0.0), _z(0.0), _Status(INVALID) {}
    TVector(double x, double y, double z) : _x(x), _y(y), _z(z), _Status(DEFAULT) {}

    // Selectors
    double X() const { return _x; }
    double Y() const { return _y; }
    double Z() const { return _z; }
    int isUnit() const { return _Status==UNIT; }
    int isDefault() const { return _Status==DEFAULT; }
    int isValid() const { return _Status!=INVALID; }

    // Change the status of a vector
    TVector &unit();
    static TVector unit(const TVector &v) { return TVector(v).unit(); }
    static TVector &unit(const TVector &v, TVector &result) { result = v; return result.unit(); }

    // Magnitude
    double mag() const { return (isValid() ? (isUnit() ? 1.0 : sqrt(sqr(X()) + sqr(Y()) + sqr(Z()))) : 0.0); }

    // Dot or scalar product
    double dot(const TVector &v) const { return ((isValid() && v.isValid()) ?
                                                     (X()*v.X() + Y()*v.Y() + Z()*v.Z()) : 0.0); }
    static double dot(const TVector &v1, const TVector &v2) { return v1.dot(v2); }

    // Distance between two vectors
    double dist(const TVector &v) const { return (*this-v).mag(); }

    // Optimised arithmetic methods
    static TVector &invert(const TVector &v1, TVector result);
    static TVector &add(const TVector &v1, const TVector &v2, TVector &result);
    static TVector &subtract(const TVector &v1, const TVector &v2, TVector &result);
    static TVector &multiply(const TVector &v1, const double &scale, TVector &result);
    static TVector &cross(const TVector &v1, const TVector &v2, TVector &result);

    // Vector arithmetic, addition, subtraction and vector product
    TVector operator-() const { return invert(*this, TVector()); }
    TVector &operator+=(const TVector &v) { return add(*this, v, *this); }
    TVector operator-(const TVector &v) const { TVector tv; return subtract(*this, v, tv); }
    TVector operator+(const TVector &v) const { TVector tv; return add(*this, v, tv); }
    TVector operator*(const double &scale) const { TVector tv; return multiply(*this, scale, tv); }
private:
    double _x, _y, _z;
    TStatus _Status;
};

#endif // TVECTOR_H
