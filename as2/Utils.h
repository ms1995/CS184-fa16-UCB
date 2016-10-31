#ifndef UTILS_H
#define UTILS_H

#include "Mat4.h"

#define DEF_W 1.0
#define DELTA 0.000001
#define abs(a) (a<0?-a:a)
#define getmin(a, b) (a<b?a:b)
#define getmax(a, b) (a>b?a:b)
#define getbound(x, a, b) getmin(getmax(x,a),b)

struct Image {
    int w, h;
    Vec3 *data;
};

inline bool isZero(double f) {
    return abs(f) < DELTA;
}

inline bool isLEZero(double f) {
    return f < DELTA;
}

inline bool isGTZero(double f) {
    return f > DELTA;
}

inline bool isGEZero(double f) {
    return isGTZero(f) || isZero(f);
}

inline double sqr(double x) {
    return x * x;
}

inline double ssqr(double a, double b) {
    return sqr(a) + sqr(b);
}

inline Mat4 getTranslationMatrix(Vec3 d) {
    double mat[] = {
            1, 0, 0, d.x,
            0, 1, 0, d.y,
            0, 0, 1, d.z,
            0, 0, 0, 1
    };
    return Mat4(mat);
}

inline Mat4 getScalingMatrix(Vec3 k) {
    double mat[] = {
            k.x, 0, 0, 0,
            0, k.y, 0, 0,
            0, 0, k.z, 0,
            0, 0, 0, 1
    };
    return Mat4(mat);
}

inline Mat4 getRotationMatrix(Vec3 r) {
    double theta = sqrt(r * r) * M_PI / 180;
    double sinx = sin(theta), cosx = 1 - cos(theta);
    r.normalize();
    double mat[] = {
            1 - ssqr(r.y, r.z) * cosx, 0 - r.z * sinx + r.x * r.y * cosx, 0 + r.y * sinx + r.x * r.z * cosx, 0,
            0 + r.z * sinx + r.x * r.y * cosx, 1 - ssqr(r.z, r.x) * cosx, 0 - r.x * sinx + r.y * r.z * cosx, 0,
            0 - r.y * sinx + r.x * r.z * cosx, 0 + r.x * sinx + r.y * r.z * cosx, 1 - ssqr(r.x, r.y) * cosx, 0,
            0, 0, 0, 1
    };
    return Mat4(mat);
}

#endif
