#ifndef AABB_H
#define AABB_H

#include "collection.h"

class aabb
{
public:
	point3 minimum, maximum;

public:
	aabb() {}
	aabb(const point3& a, const point3& b) { minimum = a; maximum = b; }

	point3 miny() const { return minimum; }
	point3 maxy() const { return maximum; }

	//bool hit(const ray& r, double t_min, double t_max) const
	//{
	//	for (int i = 0; i < 3; i++)
	//	{
	//		auto t0 = fmin((minimum[i] - r.origin()[i]) / r.direction()[i], (maximum[i] - r.origin()[i]) / r.direction()[i]);
	//		auto t1 = fmax((minimum[i] - r.origin()[i]) / r.direction()[i], (maximum[i] - r.origin()[i]) / r.direction()[i]);
	//		t_min = fmax(t0, t_min);
	//		t_max = fmin(t1, t_max);

	//		if (t_max <= t_min) return false;
	//	}

	//	return true;
	//}

	bool hit(const ray& r, double t_min, double t_max) const
	{
		for (int i = 0; i < 3; i++)
		{
			auto invD = 1.0f / r.direction()[i];
			auto t0 = (miny()[i] - r.origin()[i]) * invD;
			auto t1 = (maxy()[i] - r.origin()[i]) * invD;
			if (invD < 0.0f) std::swap(t0, t1);

			t_min = t0 > t_min ? t0 : t_min;
			t_max = t1 < t_max ? t1 : t_max;
			if (t_max <= t_min) return false;
		}

		return true;
	}
};

aabb surrounding_box(aabb box0, aabb box1)
{
	vec3 lower(
		fmin(box0.miny().x(), box1.miny().x()),
		fmin(box0.miny().y(), box1.miny().y()),
		fmin(box0.miny().z(), box1.miny().z()));

	vec3 upper(
		fmax(box0.maxy().x(), box1.maxy().x()),
		fmax(box0.maxy().y(), box1.maxy().y()),
		fmax(box0.maxy().z(), box1.maxy().z()));

	return aabb(lower, upper);
}

#endif