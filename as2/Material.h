#ifndef MATERIAL_H
#define MATERIAL_H

#include "Vec3.h"

struct Material {

    Vec3 ka, kd, ks, kr;
    double spu, spv;

    Material() : spu(0), spv(0) {};

    Material(const Vec3 &pka,
             const Vec3 &pkd,
             const Vec3 &pks,
             const Vec3 &pkr,
             const double ppu,
             const double ppv)
            : ka(pka), kd(pkd), ks(pks), kr(pkr), spu(ppu), spv(ppv) {}
};

#endif
