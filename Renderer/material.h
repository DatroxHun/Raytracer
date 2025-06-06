#ifndef MATERIAL_H
#define MATERIAL_H

#include "collection.h"
#include "hittable.h"
#include "texture.h"

class material
{
public:
	virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const = 0;
	virtual color emitted(double u, double v, const point3& p) const { return color(0.0, 0.0, 0.0); }
};

class lambertian : public material
{
public:
	lambertian(const color& a) : lambertian(make_shared<solid_color>(a)) {}
	lambertian(shared_ptr<texture> a) : albedo(a) {}

	virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override
	{
		auto scatter_direction = rec.normal + random_unit_vector();

		if (scatter_direction.near_zero()) scatter_direction = rec.normal;

		scattered = ray(rec.p, scatter_direction, r_in.time());
		attenuation = albedo->value(rec.u, rec.v, rec.p);
		return true;
	}

public:
	shared_ptr<texture> albedo;
};

class metal : public material
{
public:
	metal(const color& a, double r) : metal(make_shared<solid_color>(a), r) {}
	metal(shared_ptr<texture> a, double r) : albedo(a), roughness(fabs(r) > 1.0 ? 1.0 : fabs(r)) {}

	virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override
	{
		vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
		scattered = ray(rec.p, reflected + roughness * random_in_unit_sphere(), r_in.time());
		attenuation = albedo->value(rec.u, rec.v, rec.p);

		return dot(scattered.direction(), rec.normal) > 0;
	}

public:
	shared_ptr<texture> albedo;
	double roughness;
};

class dielectric : public material
{
public:
	dielectric(double index_of_reflection) : albedo(color(1.0)), ir(index_of_reflection) {}
	dielectric(const color& a, double index_of_reflection) : albedo(a), ir(index_of_reflection) {}

	virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override
	{
		attenuation = albedo;
		double refraction_ratio = rec.front_face ? (1.0 / ir) : ir;

		vec3 unit_direction = unit_vector(r_in.direction());
		double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
		double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

		bool cannot_refract = refraction_ratio * sin_theta > 1.0;
		vec3 scatter_direction;
		if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_double()) //if refraction is not possible
		{
			scatter_direction = reflect(unit_direction, rec.normal);
		}
		else
		{
			scatter_direction = refract(unit_direction, rec.normal, refraction_ratio);
		}

		scattered = ray(rec.p, scatter_direction, r_in.time());
		return true;
	}

public:
	color albedo;
	double ir; //index of reflection

private:
	static double reflectance(double cosine, double ref_ratio)
	{
		auto r0 = (1.0 - ref_ratio) / (1.0 + ref_ratio);
		r0 *= r0;
		return r0 + (1.0 - r0) * pow(1.0 - cosine, 5);
	}
};

class diffuse_light : public material
{
public:
	diffuse_light(shared_ptr<texture> t, double intensity = 1.0) : emit(t), intst(intensity) {}
	diffuse_light(color c, double intensity = 1.0) : emit(make_shared<solid_color>(c)), intst(intensity) {}

	virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override { return false; }
	virtual color emitted(double u, double v, const point3& p) const override
	{
		return emit->value(u, v, p) * intst;
	}

public:
	shared_ptr<texture> emit;
	double intst;
};

class isotropic : public material
{
public:
	isotropic(color color) : albedo(make_shared<solid_color>(color)) {}
	isotropic(shared_ptr<texture> texture) : albedo(texture) {}

	virtual bool scatter(const ray& r_in, const hit_record& rec, color& attenuation, ray& scattered) const override
	{
		scattered = ray(rec.p, random_in_unit_sphere(), r_in.time());
		attenuation = albedo->value(rec.u, rec.v, rec.p);
		return true;
	}

public:
	shared_ptr<texture> albedo;
};

#endif