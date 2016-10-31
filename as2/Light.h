#include "Vec3.h"

#ifndef LIGHT
#define LIGHT

enum LightType {
    Directional, Point, Ambient, Custom
};

class Light {

public:
    double falloff;
    LightType type;
    Vec3 pos, color;

    Light(const Vec3 &c) :
            color(c), type(Ambient) {}

    Light(const Vec3 &p,
          const Vec3 &c,
          const LightType plt,
          const double f = 0.0) :
            pos(p), color(c), type(plt), falloff(f) {}

};

#endif