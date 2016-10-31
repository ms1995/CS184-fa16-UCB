#include "Triangle.h"
#include "Utils.h"
#include "Mat3.h"

Triangle::Triangle(const Vec3 &va, const Vec3 &vb, const Vec3 &vc, const Material &pm)
        : a(va), b(vb), c(vc), material(pm) {
    Vec3 n = (vb - va).cross(vc - va).normalize();
    setTransMat(Mat4());
    setNormals(n, n, n);
    texture.data = NULL;
}

inline bool isSameSide(Vec3 p1, Vec3 p2, Vec3 a, Vec3 b) {
    return ((b-a).cross(p1-a)) * ((b-a).cross(p2-a)) >= 0.0;
}

bool Triangle::isHit(const Vec3 &E, const Vec3 &P, double &t, Vec3 &normal, Vec3 &newE, Vec3 &txc) const {
    Vec3 tE = rtrans * E;
    Vec3 tP = rtrans.vecConvert(P);

    // A + beta * (B-A) + gamma * (C-A) = E + t * P
    Mat3 M(b-a, c-a, -tP);
    if (!M.inverse())
        return false;
    // ans = { beta, gamma, t }
    Vec3 ans = M * (tE - a);
    double alpha = 1 - ans.x - ans.y;
    if (isLEZero(ans.x) || isLEZero(ans.y) || isLEZero(alpha) || isLEZero(t = ans.z))
        return false;
    newE = tE + t * tP;
    normal = (na * alpha) + (nb * ans.x) + (nc * ans.y);
    if (texture.data != NULL) {
        int tx_x = (int) round((alpha * texture_x.x + ans.x * texture_x.y + ans.y * texture_x.z) * (texture.w - 1));
        int tx_y = (int) round((alpha * texture_y.x + ans.x * texture_y.y + ans.y * texture_y.z) * (texture.h - 1));
        tx_x = getbound(tx_x, 0, texture.w - 1);
        tx_y = getbound(tx_y, 0, texture.h - 1);
        txc = texture.data[tx_x + tx_y * texture.w];
    }
/*
    // Intersecting with the surface?
    normal = (b - a).cross(c - a);
    double fa = (a - tE) * normal, fb = tP * normal;
    if (isZero(fb))
        return false;
    t = fa / fb;
    if (isLEZero(t))
        return false;
    newE = tE + t * tP;

    // Intersecting point inside triangle?
    if (!(isSameSide(newE, a, b, c) && isSameSide(newE, b, a, c) && isSameSide(newE, c, a, b)))
        return false;
*/
    newE = trans * newE;
    normal = trtrans.vecConvert(normal).normalize();

    return true;
}

Triangle& Triangle::setTransMat(const Mat4 &m) {
    rtrans = trans = m;
    if (!rtrans.inverse())
        rtrans = trans = Mat4();
    trtrans = rtrans;
    trtrans.transpose();

    Vec3 ta = m * a, tb = m * b, tc = m * c;
    min_v.x = getmin(ta.x, getmin(tb.x, tc.x));
    min_v.y = getmin(ta.y, getmin(tb.y, tc.y));
    min_v.z = getmin(ta.z, getmin(tb.z, tc.z));

    max_v.x = getmax(ta.x, getmax(tb.x, tc.x));
    max_v.y = getmax(ta.y, getmax(tb.y, tc.y));
    max_v.z = getmax(ta.z, getmax(tb.z, tc.z));

    return *this;
}

Triangle &Triangle::setNormals(const Vec3 &pna, const Vec3 &pnb, const Vec3 &pnc) {
    na = pna;
    nb = pnb;
    nc = pnc;

    return *this;
}

Triangle &Triangle::setTexture(const Image &tx, const Vec3 &txx, const Vec3 &txy) {
    texture = tx;
    texture_x = txx;
    texture_y = txy;
    return *this;
}
