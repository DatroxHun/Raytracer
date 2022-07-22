#ifndef CONSTANT_MEDIUM_H
#define CONSTANT_MEDIUM_H

#include "collection.h"

#include "hittable.h"
#include "material.h"
#include "texture.h"

class constant_medium : public hittable
{
public:
	constant_medium(shared_ptr<hittable> boundry, double density, shared_ptr<texture> texture)
		: bound(boundry), neg_inv_density(-1.0 / density), phase_function(make_shared<isotropic>(texture)) {}

	constant_medium(shared_ptr<hittable> boundry, double density, color color)
		: bound(boundry), neg_inv_density(-1.0 / density), phase_function(make_shared<isotropic>(color)) {}

	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;

	virtual bool bounding_box(double time0, double time1, aabb& output_box) const override
	{
		return bound->bounding_box(time0, time1, output_box);
	}

public:
	shared_ptr<hittable> bound;
	shared_ptr<material> phase_function;
	double neg_inv_density;
};

bool constant_medium::hit(const ray& r, double t_min, double t_max, hit_record& rec) const
{
	const bool enableDebug = false;
	const bool debugging = enableDebug && random_double() < .00001;

	hit_record rec1, rec2;

	if (!bound->hit(r, -infinity, infinity, rec1)) return false;
	if (!bound->hit(r, rec1.t + .0001, infinity, rec2)) return false;

	if (debugging) std::cerr << "\nt_min=" << rec1.t << ", t_max=" << rec2.t << '\n';

	rec1.t = fmax(t_min, rec1.t);
	rec2.t = fmin(t_max, rec2.t);

	if (rec1.t >= rec2.t) return false;

	rec1.t = fmax(0.0, rec1.t);

	const auto ray_length = r.direction().length();
	const auto distance_inside_boundry = (rec2.t - rec1.t) * ray_length;
	const auto hit_distance = neg_inv_density * log(random_double());

	if (hit_distance > distance_inside_boundry) return false;

	rec.t = rec1.t + hit_distance / ray_length;
	rec.p = r.at(rec.t);

	if (debugging) std::cerr << "hit_distance = " << hit_distance << '\n' << "rec.t = " << rec.t << '\n' << "rec.p = " << rec.p << '\n';

	rec.normal = vec3(1.0, 0.0, 0.0); //arbitrary
	rec.front_face = true; //also arbitrary
	rec.mat_ptr = phase_function;

	return true;
}

#endif