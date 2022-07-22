#ifndef COLOR_H
#define COLOR_H

#include "vec3.h"
#include "bitmap.h"

#include <iostream>

#define SIXTH 0.1666666666666667

void write_color(std::ostream& out, color pixel_color, int samples_per_pixel)
{
	pixel_color /= samples_per_pixel;

	//the sqrt()s are there for gamma correction where gamma = 2.0
	out << static_cast<int>(256 * clamp(sqrt(pixel_color.x()), 0.0, .999)) << ' '
		<< static_cast<int>(256 * clamp(sqrt(pixel_color.y()), 0.0, .999)) << ' '
		<< static_cast<int>(256 * clamp(sqrt(pixel_color.z()), 0.0, .999)) << '\n';
}

void write_color(BYTE* buffer, int i, color pixel_color, int samples_per_pixel)
{
	pixel_color /= samples_per_pixel;

	buffer[i + 0] = static_cast<BYTE>(256 * clamp(sqrt(pixel_color.z()), 0.0, .999));
	buffer[i + 1] = static_cast<BYTE>(256 * clamp(sqrt(pixel_color.y()), 0.0, .999));
	buffer[i + 2] = static_cast<BYTE>(256 * clamp(sqrt(pixel_color.x()), 0.0, .999));
}

void write_color_raw(BYTE* buffer, int i, color pixel_color)
{
	buffer[i + 0] = static_cast<BYTE>(pixel_color.x());
	buffer[i + 1] = static_cast<BYTE>(pixel_color.y());
	buffer[i + 2] = static_cast<BYTE>(pixel_color.z());
}

color read_color_raw(BYTE* buffer, int i)
{
	return color(buffer[i], buffer[i + 1], buffer[i + 2]);
}

color RGB2HSL(color c)
{
	double Cmax = max(c.x(), max(c.y(), c.z()));
	double Cmin = min(c.x(), min(c.y(), c.z()));
	double delta = Cmax - Cmin;

	double L = (Cmax + Cmin) / 2.0;
	double S = delta == 0 ? 0 : delta / (1.0 - fabs(2.0 * L - 1.0));
	double H;

	if (delta == 0) {
		H = 0;
	}
	else if (Cmax == c.x()) {
		H = (c.y() - c.z()) / delta;
		H += H < 0.0 ? 6.0 : 0.0;
	}
	else if (Cmax == c.y()) {
		H = (c.z() - c.x()) / delta + 2.0;
	}
	else {
		H = (c.x() - c.y()) / delta + 4.0;
	}

	return color(H / 6.0, S, L);
}

color HSL2RGB(color c)
{
	double C = (1.0 - fabs(2 * c.z() - 1.0)) * c.y();
	double X = C * (1.0 - fabs(fmod(c.x() * 6.0, 2.0) - 1.0));
	double m = c.z() - C / 2.0;

	color output;
	if (c.x() < 1.0 * SIXTH) {
		output = color(C, X, 0);
	}
	else if (c.x() < 2.0 * SIXTH) {
		output = color(X, C, 0);
	}
	else if (c.x() < 3.0 * SIXTH) {
		output = color(0, C, X);
	}
	else if (c.x() < 4.0 * SIXTH) {
		output = color(0, X, C);
	}
	else if (c.x() < 5.0 * SIXTH) {
		output = color(X, 0, C);
	}
	else {
		output = color(C, 0, X);
	}

	return output + m;
}

void saturate(color& c, double s)
{
	s = clamp(s, -1.0, 1.0);
	c = RGB2HSL(c);
	c = s < 0 ? color(c.x(), (1.0 + s) * c.y(), c.z()) : color(c.x(), (1.0 - s) * c.y() + s, c.z());
	c = HSL2RGB(c);
}

void pop_filter(color& c, double p)
{
	p = clamp(p, -1.0, 1.0);
	c ^= sqrt(1.0 + p);
	saturate(c, p);
}

#endif