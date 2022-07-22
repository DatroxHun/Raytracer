#ifndef CAMERA_H
#define CAMERA_H

#include "collection.h"

class camera
{
public:
	camera(
		point3 lookfrom,
		point3 lookat,
		vec3 vup,
		double vfov, //vertical field of view in degrees
		double aspect_ratio,
		double aperture,
		double focus_dist,
		double _time0 = 0,
		double _time1 = 0)
	{
		auto theta = deg2rad(vfov);
		auto viewport_height = 2.0 * tan(theta / 2.0);
		auto viewport_width = viewport_height * aspect_ratio;

		w = unit_vector(lookfrom - lookat);
		u = unit_vector(cross(vup, w));
		v = cross(w, u);

		origin = lookfrom;
		horizontal = focus_dist * viewport_width * u;
		vertical = focus_dist * viewport_height * v;
		lower_left_corner = origin - horizontal / 2.0 - vertical / 2.0 - focus_dist * w;

		lens_radius = aperture / 2.0;
		time0 = _time0;
		time1 = _time1;
	}

	ray get_ray(double s, double t) const
	{
		vec3 rd = lens_radius * random_in_unit_disk();
		vec3 offset = u * rd.x() + v * rd.y();

		return ray(origin + offset, lower_left_corner + s * horizontal + t * vertical - origin - offset, random_double(time0, time1));
	}

private:
	point3 origin;
	point3 lower_left_corner;
	vec3 horizontal;
	vec3 vertical;
	vec3 w, u, v;
	double lens_radius;
	double time0, time1; //shutter open/close times
};

#endif