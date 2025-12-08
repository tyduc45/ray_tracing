#ifndef RAY_H
#define RAY_H
#include "vec3.h"
class ray
{
    public:
        ray(){}
        ray(const point3D& origin, const vec3& direction): orig(origin),dir(direction) {}

        const point3D &origin() const { return orig; }
        const vec3& direction() const { return dir; }

        point3D at(double t) const{
            return orig + t * dir;
        }

    private:
        point3D orig;
        vec3    dir;
};
#endif