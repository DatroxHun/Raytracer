#ifndef AARECT_H
#define AARECT_H

#include "collection.h"
#include "hittable.h"

class xy_rect : public hittable
{
public:
	xy_rect() {}
	xy_rect(double _x0, double _x1, double _y0, double _y1, double _z, shared_ptr<material> material) : x0(_x0), x1(_x1), y0(_y0), y1(_y1), z(_z), mat(material) {}

	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;

	virtual bool bounding_box(double time0, double time1, aabb& output_box) const override
	{
		output_box = aabb(point3(x0, y0, z - .0001), point3(x1, y1, z + .0001));
		return true;
	}

public:
	shared_ptr<material> mat;
	double x0, x1, y0, y1, z;
};

class xz_rect : public hittable
{
public:
	xz_rect() {}
	xz_rect(double _x0, double _x1, double _z0, double _z1, double _y, shared_ptr<material> material) : x0(_x0), x1(_x1), z0(_z0), z1(_z1), y(_y), mat(material) {};

	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;

	virtual bool bounding_box(double time0, double time1, aabb& output_box) const override {
		output_box = aabb(point3(x0, y - .0001, z0), point3(x1, y + .0001, z1));
		return true;
	}

public:
	shared_ptr<material> mat;
	double x0, x1, z0, z1, y;
};

class yz_rect : public hittable
{
public:
	yz_rect() {}
	yz_rect(double _y0, double _y1, double _z0, double _z1, double _x, shared_ptr<material> material) : y0(_y0), y1(_y1), z0(_z0), z1(_z1), x(_x), mat(material) {};

	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;

	virtual bool bounding_box(double time0, double time1, aabb& output_box) const override {
		output_box = aabb(point3(x - .0001, y0, z0), point3(x + .0001, y1, z1));
		return true;
	}

public:
	shared_ptr<material> mat;
	double y0, y1, z0, z1, x;
};

bool xy_rect::hit(const ray& r, double t_min, double t_max, hit_record& rec) const
{
	if (r.direction().z() == 0) return false;

	auto t = (z - r.origin().z()) / r.direction().z();
	if (t < t_min || t > t_max) return false;

	auto p = r.at(t);
	if (p.x() < x0 || p.x() > x1 || p.y() < y0 || p.y() > y1) return false;

	rec.u = (p.x() - x0) / (x1 - x0);
	rec.v = (p.y() - y0) / (y1 - y0);
	rec.t = t;
	vec3 outward_normal(0.0, 0.0, 1.0);
	rec.set_face_normal(r, outward_normal);
	rec.mat_ptr = mat;
	rec.p = p;

	return true;
}

bool xz_rect::hit(const ray& r, double t_min, double t_max, hit_record& rec) const
{
	if (r.direction().y() == 0) return false;

	auto t = (y - r.origin().y()) / r.direction().y();
	if (t < t_min || t > t_max)	return false;

	auto p = r.at(t);
	if (p.x() < x0 || p.x() > x1 || p.z() < z0 || p.z() > z1) return false;

	rec.u = (p.x() - x0) / (x1 - x0);
	rec.v = (p.z() - z0) / (z1 - z0);
	rec.t = t;
	auto outward_normal = vec3(0.0, 1.0, 0.0);
	rec.set_face_normal(r, outward_normal);
	rec.mat_ptr = mat;
	rec.p = p;

	return true;
}

bool yz_rect::hit(const ray& r, double t_min, double t_max, hit_record& rec) const
{
	if (r.direction().x() == 0) return false;

	auto t = (x - r.origin().x()) / r.direction().x();
	if (t < t_min || t > t_max) return false;

	auto p = r.at(t);
	if (p.y() < y0 || p.y() > y1 || p.z() < z0 || p.z() > z1) return false;

	rec.u = (p.y() - y0) / (y1 - y0);
	rec.v = (p.z() - z0) / (z1 - z0);
	rec.t = t;
	auto outward_normal = vec3(1.0, 0.0, 0.0);
	rec.set_face_normal(r, outward_normal);
	rec.mat_ptr = mat;
	rec.p = p;

	return true;
}

#endif
