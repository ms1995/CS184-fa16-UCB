#include "Vec3.h"
#include "Utils.h"

Vec3::Vec3(const float x, const float y, const float z) : x(x), y(y), z(z) {}

Vec3::Vec3() : x(0.0f), y(0.0f), z(0.0f) {}

Vec3& Vec3::fill(const float n) {
    x = y = z = n;
    return *this;
}

Vec3 Vec3::operator-() const {
    return Vec3(-x, -y, -z);
}

Vec3 Vec3::operator+(const Vec3 &va) const {
    return Vec3(x + va.x, y + va.y, z + va.z);
}

Vec3 &Vec3::operator+=(const Vec3 &va) {
    return *this = *this + va;
}

Vec3 Vec3::operator-(const Vec3 &va) const {
    return Vec3(x - va.x, y - va.y, z - va.z);
}

Vec3& Vec3::operator-=(const Vec3 &va) {
    return *this = *this - va;
}

Vec3 Vec3::operator*(const float m) const {
    return Vec3(x * m, y * m, z * m);
}

float Vec3::operator*(const Vec3 &va) const {
    return x * va.x + y * va.y + z * va.z;
}

Vec3 Vec3::operator/(const float m) const {
    return Vec3(x / m, y / m, z / m);
}


Vec3 &Vec3::operator*=(const float m) {
    x *= m; y *= m; z *= m;
    return *this;
}

float Vec3::length() const {
    return (float) sqrt(*this * *this);
}

Vec3& Vec3::normalize() {
    float l = length();

    if (isZero(l))
        return *this;

    x /= l;
    y /= l;
    z /= l;

    return *this;
}

Vec3 Vec3::scale(const Vec3 &va) const {
    return Vec3(x * va.x, y * va.y, z * va.z);
}

float Vec3::dist(const Vec3 &va) const {
    return (*this - va).length();
}

Vec3 Vec3::cross(const Vec3 &va) const {
    return Vec3(y * va.z - z * va.y,
                z * va.x - x * va.z,
                x * va.y - y * va.x);
}

Vec3 operator*(const float m, const Vec3 &va) {
    return va * m;
}

void Vec3::print() const {
    printf("[%f, %f, %f]\n", x, y, z);
}

bool Vec3::allZeros() const {
    return isZero(x) && isZero(y) && isZero(z);
}

void Vec3::toArray(float *a) const {
    a[0] = x;
    a[1] = y;
    a[2] = z;
}

int Vec3::read(std::istream &fin) {
    int ct = 0;
    fin >> x && ++ct && fin >> y && ++ct && fin >> z && ++ct;
    return ct;
}

float Vec3::getMax() const {
    return fmaxf(x, fmaxf(y, z));
}

float Vec3::getMin() const {
    return fminf(x, fminf(y, z));
}

void Vec3::keepMin(const Vec3 &v) {
    x = fminf(x, v.x);
    y = fminf(y, v.y);
    z = fminf(z, v.z);
}

void Vec3::keepMax(const Vec3 &v) {
    x = fmaxf(x, v.x);
    y = fmaxf(y, v.y);
    z = fmaxf(z, v.z);
}

std::ostream& operator<<(std::ostream &os, const Vec3 &v) {
    return os << v.x << " " << v.y << " " << v.z;
}
