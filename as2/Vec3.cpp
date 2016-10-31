#include "Vec3.h"
#include "Utils.h"

Vec3::Vec3(const double x, const double y, const double z) : x(x), y(y), z(z) {}

Vec3::Vec3() : x(0.0), y(0.0), z(0.0) {}

Vec3& Vec3::fill(const double n) {
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

Vec3 Vec3::operator*(const double m) const {
    return Vec3(x * m, y * m, z * m);
}

double Vec3::operator*(const Vec3 &va) const {
    return x * va.x + y * va.y + z * va.z;
}

Vec3 Vec3::operator/(const double m) const {
    return Vec3(x / m, y / m, z / m);
}


Vec3 &Vec3::operator*=(const double m) {
    return *this *= m;
}

double Vec3::length() const {
    return sqrt(*this * *this);
}

Vec3& Vec3::normalize() {
    double l = length();
    double t = isZero(l) ? 0 : l;

    x /= t;
    y /= t;
    z /= t;

    return *this;
}

Vec3 Vec3::scale(const Vec3 &va) const {
    return Vec3(x * va.x, y * va.y, z * va.z);
}

double Vec3::dist(const Vec3 &va) const {
    return (*this - va).length();
}

Vec3 Vec3::cross(const Vec3 &va) const {
    return Vec3(y * va.z - z * va.y,
                z * va.x - x * va.z,
                x * va.y - y * va.x);
}

Vec3 operator*(const double m, const Vec3 &va) {
    return va * m;
}

void Vec3::print() const {
    printf("\n[%f, %f, %f]\n", x, y, z);
}

bool Vec3::allZeros() const {
    return isZero(x) && isZero(y) && isZero(z);
}

void Vec3::toArray(double *a) const {
    a[0] = x;
    a[1] = y;
    a[2] = z;
}

int Vec3::read(std::istream &fin) {
    int ct = 0;
    fin >> x && ++ct && fin >> y && ++ct && fin >> z && ++ct;
    return ct;
}

Vec4::Vec4(const double *v) {
    vec3 = Vec3(v[0], v[1], v[2]);
    extra = v[3];
}

void Vec4::toArray(double *a) const {
    vec3.toArray(a);
    a[3] = extra;
}

Vec4::Vec4(const Vec3 &vec3, const double extra) {
    this->vec3 = vec3;
    this->extra = extra;
}
