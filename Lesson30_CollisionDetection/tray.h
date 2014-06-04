#ifndef TRAY_H
#define TRAY_H

#include "tvector.h"

class TRay
{
public:
    TRay(){};
    // Line betwen two points OR point and a direction
    TRay(const TVector &point1, const TVector &point2);
    // Selectors
    TVector P() const { return _P; }
    TVector V() const { return _V; }
    int isValid() const { return V().isUnit() && P().isValid(); }
    // Distances
    double dist(const TVector &point) const;
private:
    TVector _P; // Any point on the line
    TVector _V; // Direction of the line
};

#endif // TRAY_H
