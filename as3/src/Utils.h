#ifndef UTILS_H
#define UTILS_H

#define EPSILON 0.00001f

inline bool isZero(float f) {
    return fabs(f) < EPSILON;
}

inline bool isLEZero(float f) {
    return f < EPSILON;
}

inline bool isGTZero(float f) {
    return f > EPSILON;
}

inline bool isGEZero(float f) {
    return isGTZero(f) || isZero(f);
}

inline float sqr(float x) {
    return x * x;
}

inline float ssqr(float a, float b) {
    return sqr(a) + sqr(b);
}

#endif
