#include <cstring>
#include "Mat3.h"
#include "Utils.h"

Mat3::Mat3(const double *d) {
    memcpy(data, d, sizeof(data));
}

bool Mat3::inverse() {
    double det = data[0] * (data[1 * 3 + 1] * data[2 * 3 + 2] - data[2 * 3 + 1] * data[1 * 3 + 2]) -
                 data[1] * (data[1 * 3 + 0] * data[2 * 3 + 2] - data[1 * 3 + 2] * data[2 * 3 + 0]) +
                 data[2] * (data[1 * 3 + 0] * data[2 * 3 + 1] - data[1 * 3 + 1] * data[2 * 3 + 0]);

    if (isZero(det))
        return false;

    double data_tmp[sizeof(data)];
    memcpy(data_tmp, data, sizeof(data));

    data[0 * 3 + 0] = (data_tmp[1 * 3 + 1] * data_tmp[2 * 3 + 2] - data_tmp[2 * 3 + 1] * data_tmp[1 * 3 + 2]) / det;
    data[0 * 3 + 1] = (data_tmp[0 * 3 + 2] * data_tmp[2 * 3 + 1] - data_tmp[0 * 3 + 1] * data_tmp[2 * 3 + 2]) / det;
    data[0 * 3 + 2] = (data_tmp[0 * 3 + 1] * data_tmp[1 * 3 + 2] - data_tmp[0 * 3 + 2] * data_tmp[1 * 3 + 1]) / det;
    data[1 * 3 + 0] = (data_tmp[1 * 3 + 2] * data_tmp[2 * 3 + 0] - data_tmp[1 * 3 + 0] * data_tmp[2 * 3 + 2]) / det;
    data[1 * 3 + 1] = (data_tmp[0 * 3 + 0] * data_tmp[2 * 3 + 2] - data_tmp[0 * 3 + 2] * data_tmp[2 * 3 + 0]) / det;
    data[1 * 3 + 2] = (data_tmp[1 * 3 + 0] * data_tmp[0 * 3 + 2] - data_tmp[0 * 3 + 0] * data_tmp[1 * 3 + 2]) / det;
    data[2 * 3 + 0] = (data_tmp[1 * 3 + 0] * data_tmp[2 * 3 + 1] - data_tmp[2 * 3 + 0] * data_tmp[1 * 3 + 1]) / det;
    data[2 * 3 + 1] = (data_tmp[2 * 3 + 0] * data_tmp[0 * 3 + 1] - data_tmp[0 * 3 + 0] * data_tmp[2 * 3 + 1]) / det;
    data[2 * 3 + 2] = (data_tmp[0 * 3 + 0] * data_tmp[1 * 3 + 1] - data_tmp[1 * 3 + 0] * data_tmp[0 * 3 + 1]) / det;

    return true;
}

Vec3 Mat3::operator*(const Vec3 &v) const {
    double in[3], ret[3];
    v.toArray(in);
    for (int i = 0; i < 3; ++i) {
        ret[i] = 0.0;
        for (int j = 0; j < 3; ++j)
            ret[i] += data[i * 3 + j] * in[j];
    }
    return Vec3(ret[0], ret[1], ret[2]);
}

Mat3::Mat3(const Vec3 &va, const Vec3 &vb, const Vec3 &vc) {
    data[0] = va.x; data[1] = vb.x; data[2] = vc.x;
    data[3] = va.y; data[4] = vb.y; data[5] = vc.y;
    data[6] = va.z; data[7] = vb.z; data[8] = vc.z;
}
