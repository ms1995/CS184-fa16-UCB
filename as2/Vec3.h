#include <math.h>
#include <stdio.h>
#include <fstream>

#ifndef VEC3
#define VEC3

class Vec3 {

public:
    double x, y, z;

public:
    Vec3(const double x, const double y, const double z);
    Vec3();

    Vec3& fill(const double n);
    double length() const;
    Vec3& normalize();
    void print() const;
    int read(std::istream &);

    double dist(const Vec3 &va) const;
    Vec3 scale(const Vec3 &va) const;
    Vec3 cross(const Vec3 &va) const;
    bool allZeros() const;
    void toArray(double *) const;

    Vec3 operator-() const;
    Vec3 operator+(const Vec3 &va) const;
    Vec3 operator-(const Vec3 &va) const;
    double operator*(const Vec3 &va) const;
    Vec3 operator*(const double m) const;
    Vec3 operator/(const double m) const;

    Vec3& operator+=(const Vec3 &va);
    Vec3& operator-=(const Vec3 &va);
    Vec3& operator*=(const double m);

};

Vec3 operator*(const double m, const Vec3 &va);

struct Vec4 {
    Vec3 vec3;
    double extra;

    Vec4(const double *);
    Vec4(const Vec3 &, const double);
    void toArray(double *) const;
};

#endif