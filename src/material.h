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

class dielectric : public material
{
public:
    dielectric(double refraction_index) : refraction_index(refraction_index) {}
    bool scatter(const ray &r_in, const hit_record &rec, color &attenuation, ray &scattered)
        const override
    {
        attenuation = color(1.0, 1.0, 1.0);
        double ri = rec.front_face ? (1.0 / refraction_index) : refraction_index;

        vec3 unit_direction = unit_vector(r_in.direction());

        double cos_theta = std::fmin(dot(-unit_direction, rec.normal), 1.0);
        double sin_theta = std::sqrt(1.0 - cos_theta * cos_theta);
        vec3 direction;
        // the light reflect when no-solotion, or it has some probability to be reflected by the glass.
        if (ri * sin_theta > 1.0 || reflectance(cos_theta, ri) > random_double())
        {
            // must reflect
            direction = mirror_reflect(unit_direction,rec.normal);
        }
        else
        {
            // can refract
            direction = refract(unit_direction, rec.normal, ri);
        }

        scattered = ray(rec.p,direction);
        return true;
    }

private:
    double refraction_index;
    // for glass , part of the light reflect and part of it refract , this calculates the ratio
    static double reflectance(double cosine, double refraction_index)
    {
        // Use Schlick's approximation for reflectance.
        auto r0 = (1 - refraction_index) / (1 + refraction_index);
        r0 = r0 * r0;
        return r0 + (1 - r0) * std::pow((1 - cosine), 5);
    }
};

#endif