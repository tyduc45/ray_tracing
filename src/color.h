#ifndef COLOR_H
#define COLOR_H

#include "raytracer.h"


using color = vec3;

inline double linear_to_gamma(double linear_component){
    if(linear_component > 0)
        return std::sqrt(linear_component);
    return 0;
}

void write_color(std::ostream& out, const color& pixel_color){
    // the [0,1] component values
    auto r = pixel_color.x();
    auto g = pixel_color.y();
    auto b = pixel_color.z();

    // Apply a linear to gamma transform for gamma 2
    r = linear_to_gamma(r);
    g = linear_to_gamma(g);
    b = linear_to_gamma(b);

    // translate the [0,1] RGB component values to [0,255] RGB values
    static const interval instensity(0.000, 0.999);   // to avoid unessary ctor call
    int rbyte = int(256 * instensity.clamp(r));
    int gbyte = int(256 * instensity.clamp(g));
    int bbyte = int(256 * instensity.clamp(b));

    //Write out the pixel color components.
    out << rbyte << ' ' << gbyte << ' ' << bbyte << '\n';
}

#endif