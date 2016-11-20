#include <math.h>
#include <stdio.h>
#include <fstream>

#ifndef VEC3
#define VEC3

class Vec3 {

public:
    float x, y, z;

public:
    Vec3(const float x, const float y, const float z);
    Vec3();

    Vec3& fill(const float n);
    float length() const;
    Vec3& normalize();
    void print() const;
    int read(std::istream &);

    float dist(const Vec3 &va) const;
    Vec3 scale(const Vec3 &va) const;
    Vec3 cross(const Vec3 &va) const;
    bool allZeros() const;
    void toArray(float *) const;
    float getMax() const;
    float getMin() const;
    void keepMax(const Vec3 &);
    void keepMin(const Vec3 &);

    Vec3 operator-() const;
    Vec3 operator+(const Vec3 &va) const;
    Vec3 operator-(const Vec3 &va) const;
    float operator*(const Vec3 &va) const;
    Vec3 operator*(const float m) const;
    Vec3 operator/(const float m) const;

    Vec3& operator+=(const Vec3 &va);
    Vec3& operator-=(const Vec3 &va);
    Vec3& operator*=(const float m);

};

Vec3 operator*(const float m, const Vec3 &va);
std::ostream& operator<<(std::ostream &, const Vec3 &);

#endif