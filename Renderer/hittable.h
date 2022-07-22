#ifndef HITTABLE_H
#define HITTABLE_H

#include "ray.h"
#include "collection.h"
#include "aabb.h"

class material;

struct hit_record
{
	point3 p;
	vec3 normal;
	shared_ptr<material> mat_ptr;
	double t;
	double u, v;
	bool front_face;

	inline void set_face_normal(const ray& r, const vec3& outward_normal)
	{
		front_face = dot(r.direction(), outward_normal) < 0;
		normal = front_face ? outward_normal : -outward_normal;
	}
};

class hittable
{
public:
	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const = 0;
	virtual bool bounding_box(double time0, double time1, aabb& output_box) const = 0;
};

class translate : public hittable
{
public:
	translate(shared_ptr<hittable> object, const vec3& displacement) : obj(object), offset(displacement) {}

	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;
	virtual bool bounding_box(double time0, double time1, aabb& output_box) const override;

public:
	shared_ptr<hittable> obj;
	vec3 offset;
};

bool translate::hit(const ray& r, double t_min, double t_max, hit_record& rec) const
{
	ray moved_ray(r.origin() - offset, r.direction(), r.time());
	if (!obj->hit(moved_ray, t_min, t_max, rec)) return false;

	rec.p += offset;
	rec.set_face_normal(moved_ray, rec.normal);

	return true;
}

bool translate::bounding_box(double time0, double time1, aabb& output_box) const
{
	if (!obj->bounding_box(time0, time1, output_box)) return false;

	output_box = aabb(output_box.miny() + offset, output_box.maxy() + offset);
	return true;
}

class rotate_y : public hittable
{
public:
	rotate_y(shared_ptr<hittable> object, double angle);

	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;
	virtual bool bounding_box(double time0, double time1, aabb& output_box) const override
	{
		output_box = bbox;
		return hasbox;
	}

public:
	shared_ptr<hittable> obj;
	double sin_theta;
	double cos_theta;
	bool hasbox;
	aabb bbox;
};

rotate_y::rotate_y(shared_ptr<hittable> object, double angle) : obj(object)
{
	auto rad = deg2rad(angle);
	sin_theta = sin(rad);
	cos_theta = cos(rad);
	hasbox = obj->bounding_box(0.0, 1.0, bbox);

	point3 minimum(infinity);
	point3 maximum(-infinity);

	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 2; j++)
		{
			for (int k = 0; k < 2; k++)
			{
				auto x = i * bbox.maxy().x() + (1 - i) * bbox.miny().x();
				auto y = j * bbox.maxy().y() + (1 - j) * bbox.miny().y();
				auto z = k * bbox.maxy().z() + (1 - k) * bbox.miny().z();

				auto newX = cos_theta * x + sin_theta * z;
				auto newZ = -sin_theta * x + cos_theta * z;

				vec3 curr(newX, y, newZ);

				for (int c = 0; c < 3; c++)
				{
					minimum[c] = fmin(minimum[c], curr[c]);
					maximum[c] = fmax(maximum[c], curr[c]);
				}
			}
		}
	}

	bbox = aabb(minimum, maximum);
}

bool rotate_y::hit(const ray& r, double t_min, double t_max, hit_record& rec) const
{
	auto origin = r.origin();
	auto direction = r.direction();

	origin[0] = cos_theta * r.origin()[0] - sin_theta * r.origin()[2];
	origin[2] = sin_theta * r.origin()[0] + cos_theta * r.origin()[2];

	direction[0] = cos_theta * r.direction()[0] - sin_theta * r.direction()[2];
	direction[2] = sin_theta * r.direction()[0] + cos_theta * r.direction()[2];

	ray rotated_ray(origin, direction, r.time());

	if (!obj->hit(rotated_ray, t_min, t_max, rec)) return false;

	auto p = rec.p;
	auto normal = rec.normal;

	p[0] = cos_theta * rec.p[0] + sin_theta * rec.p[2];
	p[2] = -sin_theta * rec.p[0] + cos_theta * rec.p[2];

	normal[0] = cos_theta * rec.normal[0] + sin_theta * rec.normal[2];
	normal[2] = -sin_theta * rec.normal[0] + cos_theta * rec.normal[2];

	rec.p = p;
	rec.set_face_normal(rotated_ray, normal);

	return true;
}

#endif