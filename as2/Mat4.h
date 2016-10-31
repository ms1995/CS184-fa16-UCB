#include "Vec3.h"

#ifndef MAT4
#define MAT4

class Mat4 {

public:
    double data[16];

    Mat4();
    Mat4(const double *);
    Mat4 operator* (const Mat4 &) const;
    Vec4 operator* (const Vec4 &) const;
    Vec3 operator* (const Vec3 &) const;
    Vec3 vecConvert(const Vec3 &) const;
    Mat4& transpose();
    // double det() const;
    bool inverse();
    void print();

};

#endif