#include "Vec3.h"
#include "Mat4.h"
#include "Material.h"

#ifndef SPHERE
#define SPHERE

class Sphere {

public:
    double R;
    Vec3 C;
    Mat4 trans, rtrans, trtrans;
    Vec3 min_v, max_v;

    Material material;

    Sphere(const Vec3 &, const double, const Material &);

    Sphere& setTransMat(const Mat4 &);
    bool isHit(const Vec3 &, const Vec3 &, double &, Vec3 &, Vec3 &) const;

};

#endif