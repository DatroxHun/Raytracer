#ifndef RECT_H
#define RECT_H

#include "collection.h"
#include "hittable.h"

class rect : public hittable
{
public:
	rect() {}
	rect(point3 position, vec3 i_vect, vec3 j_vect, shared_ptr<material> material)
		: pos(position), i(unit_vector(i_vect)), abs_i(i_vect.length()), j(unit_vector(j_vect)), abs_j(j_vect.length()), mat(material), outward_normal(vec3(1.0))
	{
		i *= -1.0;
		j *= -1.0;
		std::swap(i.e[1], i.e[2]);
		std::swap(j.e[1], j.e[2]);
		outward_normal = cross(i, j);
	}

	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;
	virtual bool bounding_box(double time0, double time1, aabb& output_box) const override;

public:
	point3 pos;
	vec3 i, j;
	shared_ptr<material> mat;
	double abs_i, abs_j;
	vec3 outward_normal;
};

bool rect::hit(const ray& r, double t_min, double t_max, hit_record& rec) const
{
	vec3 ray_dir = unit_vector(r.direction());
	vec3 relative_pos = r.origin() - pos;

	double under = dot(-ray_dir, outward_normal);
	if (under == 0) return false;

	auto t = dot(outward_normal, relative_pos) / (under * r.direction().length());
	auto v = dot(cross(-ray_dir, j), relative_pos) / under;
	auto u = dot(cross(i, -ray_dir), relative_pos) / under;
	if (t < t_min || t > t_max || u < 0.0 || u > abs_i || v < 0.0 || v > abs_j) return false;

	rec.t = t;
	rec.p = r.at(rec.t);
	rec.set_face_normal(r, unit_vector(outward_normal));
	rec.u = u / abs_i;
	if (!rec.front_face) rec.u = 1.0 - rec.u;
	rec.v = v / abs_j;
	rec.mat_ptr = mat;

	return true;
}

bool rect::bounding_box(double time0, double time1, aabb& output_box) const
{
	point3 corner0 = pos;
	point3 corner1 = pos + i + j;

	output_box = surrounding_box(aabb(corner0, corner0), aabb(corner1, corner1));
	if (output_box.maxy().x() == output_box.miny().x()) output_box.maximum[0] += .0001;
	if (output_box.maxy().y() == output_box.miny().y()) output_box.maximum[1] += .0001;
	if (output_box.maxy().z() == output_box.miny().z()) output_box.maximum[2] += .0001;

	return true;
}

#endif