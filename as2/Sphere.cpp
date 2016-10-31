#include <cmath>
#include "Sphere.h"
#include "Utils.h"

Sphere::Sphere(const Vec3 &c, const double r, const Material &pm)
        : C(c), R(r), material(pm) { setTransMat(Mat4()); }

bool Sphere::isHit(const Vec3 &E, const Vec3 &P, double &t, Vec3 &normal, Vec3 &newE) const {
    Vec3 tE = rtrans * E;
    Vec3 tP = rtrans.vecConvert(P);

    Vec3 A = tP, B = tE - C;
    double a = A * A, b = 2 * A * B, c = B * B - R * R;
    double delta = b * b - 4 * a * c;
    if (isLEZero(delta))
        return false;

    delta = sqrt(delta);
    t = -(delta + b) / 2 / a;
    if (isLEZero(t))
        return false;

    newE = tE + t * tP;
    normal = newE - C;

    newE = trans * newE;
    normal = trtrans.vecConvert(normal).normalize();

    return true;
}

Sphere& Sphere::setTransMat(const Mat4 &m) {
    rtrans = trans = m;
    if (!rtrans.inverse())
        rtrans = trans = Mat4();
    trtrans = rtrans;
    trtrans.transpose();

    Vec3 r;
    double span;
    Mat4 mat = m * getTranslationMatrix(C);

    r = Vec3(mat.data[0], mat.data[1], mat.data[2]);
    span = R * sqrt(r * r);
    min_v.x = mat.data[3] - span;
    max_v.x = mat.data[3] + span;

    r = Vec3(mat.data[4], mat.data[5], mat.data[6]);
    span = R * sqrt(r * r);
    min_v.y = mat.data[7] - span;
    max_v.y = mat.data[7] + span;

    r = Vec3(mat.data[8], mat.data[9], mat.data[10]);
    span = R * sqrt(r * r);
    min_v.z = mat.data[11] - span;
    max_v.z = mat.data[11] + span;

    return *this;
}
