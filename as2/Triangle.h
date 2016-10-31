#ifndef TRIANGLE
#define TRIANGLE

#include "Vec3.h"
#include "Mat4.h"
#include "Material.h"
#include "Utils.h"

class Triangle {

public:
    Vec3 a, b, c;
    Vec3 na, nb, nc;

    Mat4 trans, rtrans, trtrans;
    Vec3 min_v, max_v;

    Vec3 texture_x, texture_y;
    Image texture;
    Material material;

    Triangle(const Vec3 &va, const Vec3 &vb, const Vec3 &vc, const Material &);

    Triangle& setTransMat(const Mat4 &);
    Triangle& setNormals(const Vec3 &, const Vec3 &, const Vec3 &);
    Triangle& setTexture(const Image &, const Vec3 &, const Vec3 &);
    bool isHit(const Vec3 &, const Vec3 &, double &, Vec3 &, Vec3 &, Vec3 &) const;

};

#endif
