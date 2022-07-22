#ifndef HITTABLE_LIST_H
#define HITTABLE_LIST_H

#include "hittable.h"

#include <memory>
#include <vector>

using std::shared_ptr;
using std::make_shared;

class hittable_list : public hittable
{
public:
	hittable_list() {}
	hittable_list(shared_ptr<hittable> object) { add(object); }

	void clear() { objects.clear(); }
	void add(shared_ptr<hittable> object) { objects.push_back(object); }

	virtual bool hit(const ray& r, double t_min, double t_max, hit_record& rec) const override;
	virtual bool bounding_box(double time0, double time1, aabb& output_box) const override;

public:
	std::vector<shared_ptr<hittable>> objects;
};

bool hittable_list::hit(const ray& r, double t_min, double t_max, hit_record& rec) const
{
	hit_record best_rec;
	bool global_hit = false; //false if ray doesn't hit anything
	auto closest_hit = t_max;

	for (const auto& obj : objects)
	{
		if (obj->hit(r, t_min, closest_hit, best_rec))
		{
			global_hit = true;
			closest_hit = best_rec.t;
			rec = best_rec;
		}
	}

	return global_hit;
}

bool hittable_list::bounding_box(double time0, double time1, aabb& output_box) const
{
	if (objects.empty()) return false;

	aabb current_box;
	bool first_box = true;

	for (const auto& object : objects)
	{
		if (!object->bounding_box(time0, time1, current_box)) return false;
		output_box = first_box ? current_box : surrounding_box(output_box, current_box);
		first_box = false;
	}

	return true;
}

#endif