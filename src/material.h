#ifndef MATERIAL_H
#define MATERIAL_H


#include "hittable.h"  // for hit_record

class material
{
public:
    virtual ~material() = default;

    virtual bool scatter(
        const ray &r_in, const hit_record &rec, color &attenuation, ray &scattered) const
    {
        return false;
    }
};

class lambertian : public material
{
public:
    lambertian(const color &albedo) : albedo(albedo) {}
    // here we assume all light are being reflected
    // if only part of the light were being reflected, we need to let them to have the same effect compared to the original light
    // to achieve this we will let albedo / R, where R is the ratio of the reflected lights.
    // e.g red light : N * reflect_ratio * new_albedo.r = N * albedo.r  => new_albedo.r = albedo.r / reflect_ratio, same for green and blue.
    bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered)
        const override
    {
        auto scatter_direction = rec.normal + random_unit_vector();
        // avoid random_unit_vector() = - rec.normal
        if(scatter_direction.near_zero()){ 
            scatter_direction = rec.normal;
        }
        scattered = ray(rec.p, scatter_direction);
        attenuation = albedo;
        return true;
    }

private:
    color albedo;  // object color
};

class metal : public material
{
public:
    metal(const color &albedo,double fuzz) : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1) {}
    bool scatter(const ray &r_in, const hit_record &rec, color &attenuation, ray &scattered)
        const override
    {
        auto reflect_direction = mirror_reflect(r_in.direction(), rec.normal);
        // avoid random_unit_vector() = - rec.normal
        reflect_direction = unit_vector(reflect_direction) + (fuzz * random_unit_vector()); // fuzzy matel effect
        scattered = ray(rec.p, reflect_direction);
        attenuation = albedo;
        return (dot(rec.normal,scattered.direction()) > 0);
    }

private:
    color albedo; // object color
    double fuzz;
};

#endif