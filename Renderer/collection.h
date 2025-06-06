#ifndef COLLECTION_H
#define COLLECTION_H

#include <cmath>
#include <limits>
#include <memory>
#include <cstdlib>


// Usings
using std::shared_ptr;
using std::make_shared;
using std::sqrt;


// Constants
const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;


// Utility Functions
inline double deg2rad(double deg)
{
	return deg / 180.0 * pi;
}

inline double rad2deg(double rad)
{
	return rad / pi * 180.0;
}

inline double random_double()
{
	return rand() / (RAND_MAX + 1.0);
}

inline double random_double(double min, double max)
{
	return min + (max - min) * random_double();
}

inline int random_int(int min, int max) //inclusive boundries
{
	return static_cast<int>(random_double(min, max + 1));
}

inline double clamp(double x, double min, double max)
{
	if (x < min) return min;
	if (x > max) return max;
	return x;
}


// Common Headers
#include "ray.h"
#include "vec3.h"

#endif
