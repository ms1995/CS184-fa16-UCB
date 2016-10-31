#ifndef MAT3_H
#define MAT3_H


#include "Vec3.h"

class Mat3 {

public:
    double data[9];

    Mat3(const double *);
    Mat3(const Vec3 &, const Vec3 &, const Vec3 &);

    Vec3 operator*(const Vec3 &) const;

    bool inverse();

};


#endif