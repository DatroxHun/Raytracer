#ifndef TEXTURE_H
#define TEXTURE_H

#include "collection.h"
#include "perlin.h"

#include <iostream>

#ifdef _MSC_VER
#pragma warning (push, 0)
#endif

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef _MSC_VER
#pragma warning (pop)
#endif


class texture
{
public:
	virtual color value(double u, double v, const point3& p) const = 0;
};

class solid_color : public texture
{
public:
	solid_color() {}
	solid_color(color c) : color_value(c) {}
	solid_color(double red, double green, double blue) : solid_color(color(red, green, blue)) {}

	virtual color value(double u, double v, const point3& p) const override
	{
		return color_value;
	}

private:
	color color_value;
};

class checker_texture : public texture
{
public:
	checker_texture() {}
	checker_texture(shared_ptr<texture> _even, shared_ptr<texture> _odd) : even(_even), odd(_odd) {}
	checker_texture(color c1, color c2) : even(make_shared<solid_color>(c1)), odd(make_shared<solid_color>(c2)) {}

	virtual color value(double u, double v, const point3& p) const override
	{
		auto sines = sin(10 * p.x()) * sin(10 * p.y()) * sin(10 * p.z());
		if (sines < 0)
		{
			return odd->value(u, v, p);
		}
		else
		{
			return even->value(u, v, p);
		}
	}

public:
	shared_ptr<texture> even, odd;
};

class noise_texture : public texture
{
public:
	noise_texture() : scale(1.0) {}
	noise_texture(double sc) : scale(sc) {}

	virtual color value(double u, double v, const point3& p) const override
	{
		return color(1.0, 1.0, 1.0) * .5 * (1 + sin(scale * p.z() + 100 * noise.turb(0.25 * p)));
	}

public:
	perlin noise;
	double scale;
};

class image_texture : public texture
{
public:
	const static int bytes_per_pixel = 3;

	image_texture() : data(nullptr), width(0), height(0), bytes_per_scanline(0), mod(0) {}
	image_texture(const char* filename, double modifier = 0) : mod(modifier)
	{
		auto components_per_pixel = bytes_per_pixel;

		data = stbi_load(filename, &width, &height, &components_per_pixel, components_per_pixel);

		if (!data)
		{
			std::cerr << "ERROR: Could not load texture image " << filename << ".\n";
			width = height = 0;
		}

		bytes_per_scanline = bytes_per_pixel * width;
	}

	~image_texture()
	{
		delete data;
	}

	virtual color value(double u, double v, const vec3& p) const override
	{
		if (data == nullptr) return color(0.0, 1.0, 1.0);

		u = clamp(u, 0.0, 1.0);
		v = 1.0 - clamp(v, 0.0, 1.0);

		auto x = static_cast<int>(u * width);
		auto y = static_cast<int>(v * height);
		x = min(x, width - 1);
		y = min(y, height - 1);

		const auto color_scale = 1.0 / 255.0;
		auto pixel = data + y * bytes_per_scanline + x * bytes_per_pixel;

		color output(pixel[0] * color_scale, pixel[1] * color_scale, pixel[2] * color_scale);
		if (mod == 0) return output;

		pop_filter(output, mod);
		return output;
	}

private:
	unsigned char* data;
	int width, height;
	int bytes_per_scanline;
	double mod;
};

#endif