#include <cstring>
#include "Mat4.h"
#include "Utils.h"

Mat4::Mat4(const double *d) {
    memcpy(data, d, sizeof(data));
}

Mat4 Mat4::operator*(const Mat4 &m) const {
    double ret[16];
    for (int i = 0; i < 16; ++i) {
        ret[i] = 0.0;
        for (int j = 0; j < 4; ++j)
            ret[i] += data[i / 4 * 4 + j] * m.data[j * 4 + i % 4];
    }
    return Mat4(ret);
}

Vec4 Mat4::operator*(const Vec4 &v) const {
    double in[4], ret[4];
    v.toArray(in);
    for (int i = 0; i < 4; ++i) {
        ret[i] = 0.0;
        for (int j = 0; j < 4; ++j)
            ret[i] += data[i * 4 + j] * in[j];
    }
    return Vec4(ret);
}

Mat4::Mat4() {
    for (int i = 0; i < 16; ++i)
        data[i] = i / 4 == i % 4;
}

Mat4& Mat4::transpose() {
    double tmp;
    int i, j;
    for (i = 0; i < 4; ++i)
        for (j = i + 1; j < 4; ++j) {
            tmp = data[i * 4 + j];
            data[i * 4 + j] = data[j * 4 + i];
            data[j * 4 + i] = tmp;
        }
    return *this;
}

bool Mat4::inverse() {
    double inv[16], det;
    int i;

    inv[0] = data[5] * data[10] * data[15] -
             data[5] * data[11] * data[14] -
             data[9] * data[6] * data[15] +
             data[9] * data[7] * data[14] +
             data[13] * data[6] * data[11] -
             data[13] * data[7] * data[10];

    inv[4] = -data[4] * data[10] * data[15] +
             data[4] * data[11] * data[14] +
             data[8] * data[6] * data[15] -
             data[8] * data[7] * data[14] -
             data[12] * data[6] * data[11] +
             data[12] * data[7] * data[10];

    inv[8] = data[4] * data[9] * data[15] -
             data[4] * data[11] * data[13] -
             data[8] * data[5] * data[15] +
             data[8] * data[7] * data[13] +
             data[12] * data[5] * data[11] -
             data[12] * data[7] * data[9];

    inv[12] = -data[4] * data[9] * data[14] +
              data[4] * data[10] * data[13] +
              data[8] * data[5] * data[14] -
              data[8] * data[6] * data[13] -
              data[12] * data[5] * data[10] +
              data[12] * data[6] * data[9];

    inv[1] = -data[1] * data[10] * data[15] +
             data[1] * data[11] * data[14] +
             data[9] * data[2] * data[15] -
             data[9] * data[3] * data[14] -
             data[13] * data[2] * data[11] +
             data[13] * data[3] * data[10];

    inv[5] = data[0] * data[10] * data[15] -
             data[0] * data[11] * data[14] -
             data[8] * data[2] * data[15] +
             data[8] * data[3] * data[14] +
             data[12] * data[2] * data[11] -
             data[12] * data[3] * data[10];

    inv[9] = -data[0] * data[9] * data[15] +
             data[0] * data[11] * data[13] +
             data[8] * data[1] * data[15] -
             data[8] * data[3] * data[13] -
             data[12] * data[1] * data[11] +
             data[12] * data[3] * data[9];

    inv[13] = data[0] * data[9] * data[14] -
              data[0] * data[10] * data[13] -
              data[8] * data[1] * data[14] +
              data[8] * data[2] * data[13] +
              data[12] * data[1] * data[10] -
              data[12] * data[2] * data[9];

    inv[2] = data[1] * data[6] * data[15] -
             data[1] * data[7] * data[14] -
             data[5] * data[2] * data[15] +
             data[5] * data[3] * data[14] +
             data[13] * data[2] * data[7] -
             data[13] * data[3] * data[6];

    inv[6] = -data[0] * data[6] * data[15] +
             data[0] * data[7] * data[14] +
             data[4] * data[2] * data[15] -
             data[4] * data[3] * data[14] -
             data[12] * data[2] * data[7] +
             data[12] * data[3] * data[6];

    inv[10] = data[0] * data[5] * data[15] -
              data[0] * data[7] * data[13] -
              data[4] * data[1] * data[15] +
              data[4] * data[3] * data[13] +
              data[12] * data[1] * data[7] -
              data[12] * data[3] * data[5];

    inv[14] = -data[0] * data[5] * data[14] +
              data[0] * data[6] * data[13] +
              data[4] * data[1] * data[14] -
              data[4] * data[2] * data[13] -
              data[12] * data[1] * data[6] +
              data[12] * data[2] * data[5];

    inv[3] = -data[1] * data[6] * data[11] +
             data[1] * data[7] * data[10] +
             data[5] * data[2] * data[11] -
             data[5] * data[3] * data[10] -
             data[9] * data[2] * data[7] +
             data[9] * data[3] * data[6];

    inv[7] = data[0] * data[6] * data[11] -
             data[0] * data[7] * data[10] -
             data[4] * data[2] * data[11] +
             data[4] * data[3] * data[10] +
             data[8] * data[2] * data[7] -
             data[8] * data[3] * data[6];

    inv[11] = -data[0] * data[5] * data[11] +
              data[0] * data[7] * data[9] +
              data[4] * data[1] * data[11] -
              data[4] * data[3] * data[9] -
              data[8] * data[1] * data[7] +
              data[8] * data[3] * data[5];

    inv[15] = data[0] * data[5] * data[10] -
              data[0] * data[6] * data[9] -
              data[4] * data[1] * data[10] +
              data[4] * data[2] * data[9] +
              data[8] * data[1] * data[6] -
              data[8] * data[2] * data[5];

    det = data[0] * inv[0] + data[1] * inv[4] + data[2] * inv[8] + data[3] * inv[12];

    if (isZero(det))
        return false;

    det = 1.0 / det;

    for (i = 0; i < 16; ++i)
        data[i] = inv[i] * det;

    return true;
}

void Mat4::print() {
    int i, j;
    printf("\n");
    for (i = 0; i < 4; ++i) {
        printf("| ");
        for (j = 0; j < 4; ++j)
            printf("%f ", data[i * 4 + j]);
        printf("|\n");
    }
}

Vec3 Mat4::operator*(const Vec3 &v3) const {
    Vec4 v4(v3, DEF_W);
    return (*this * v4).vec3;
}

Vec3 Mat4::vecConvert(const Vec3 &v3) const {
    Vec4 v4(v3, 0);
    return (*this * v4).vec3;
}
