#ifndef COLOR_H
#define COLOR_H

#include "interval.h"
#include "vec3.h"

using color = vec3;

void write_color(std::ostream& out, const color& pixel_color){
    // the [0,1] component values
    auto r = pixel_color.x();
    auto g = pixel_color.y();
    auto b = pixel_color.z();

    // translate the [0,1] RGB component values to [0,255] RGB values
    static const interval instensity(0.000, 0.999);   // to avoid unessary ctor call
    int rbyte = int(256 * instensity.clamp(r));
    int gbyte = int(256 * instensity.clamp(g));
    int bbyte = int(256 * instensity.clamp(b));

    //Write out the pixel color components.
    out << rbyte << ' ' << gbyte << ' ' << bbyte << '\n';
}

#endif