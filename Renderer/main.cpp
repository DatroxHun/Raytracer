#include "collection.h"

#include "camera.h"
#include "color.h"
#include "bvh.h"

#include "hittable_list.h"
#include "material.h"
#include "sphere.h"
#include "moving_sphere.h"
#include "rect.h"
#include "aarect.h"
#include "box.h"
#include "constant_medium.h"

#include <iostream>
#include <sstream>
#include <shlobj.h>
#include <ctime>
#include "bitmap.h"
using std::string;

#include <thread>
#include <vector>
using std::thread;
using std::vector;
using std::atomic;

#include <chrono>
using namespace std::chrono;

void write_status(steady_clock::time_point& start, int step, int maxStep)
{
	auto duration = duration_cast<microseconds>(high_resolution_clock::now() - start);
	double progress = (double)step / (double)maxStep;

	string s(7, '\0');
	auto written = std::snprintf(&s[0], s.size(), "%.3f", progress * 100.0);
	s.resize(written);
	std::cerr << "\rRender Progress: " << s << "%; "
		<< "Expected Remaining Render Duration : " << floor(duration.count() / 1000000 * (1.0 - progress) / progress) << "s       " << std::flush;
}

void report_status(atomic<bool>* rep, steady_clock::time_point start, atomic<int>* step, int maxStep)
{
	while (rep->load())
	{
		write_status(start, step->load(), maxStep);
		std::this_thread::sleep_for(milliseconds(500));
	}

	return;
}

color ray_color(const ray& r, const color& background, const hittable& world, int depth)
{
	if (depth <= 0) return color(0.0);

	hit_record rec;
	if (!world.hit(r, .001, infinity, rec)) return background;

	ray scattered;
	color attenuation;
	color emitted = rec.mat_ptr->emitted(rec.u, rec.v, rec.p);

	if (!rec.mat_ptr->scatter(r, rec, attenuation, scattered)) return emitted;

	return emitted + attenuation * ray_color(scattered, background, world, depth - 1);
}

hittable_list random_scene() {
	hittable_list world;

	auto checker = make_shared<checker_texture>(color(.2, .3, .1), color(.9, .9, .9));
	world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, make_shared<lambertian>(checker)));

	for (int a = -11; a < 11; a++) {
		for (int b = -11; b < 11; b++) {
			auto choose_mat = random_double();
			point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

			if ((center - point3(4, 0.2, 0)).length() > 0.9) {
				shared_ptr<material> sphere_material;

				if (choose_mat < 0.8) {
					// diffuse
					auto albedo = color::random() * color::random();
					sphere_material = make_shared<lambertian>(albedo);
					auto center2 = center + vec3(0, random_double(0, .5), 0);
					world.add(make_shared<moving_sphere>(
						center, center2, 0.0, 1.0, 0.2, sphere_material));
				}
				else if (choose_mat < 0.95) {
					// metal
					auto albedo = color::random(0.5, 1);
					auto fuzz = random_double(0, 0.5);
					sphere_material = make_shared<metal>(albedo, fuzz);
					world.add(make_shared<sphere>(center, 0.2, sphere_material));
				}
				else {
					// glass
					sphere_material = make_shared<dielectric>(color(.95, .95, .95), 1.5);
					world.add(make_shared<sphere>(center, 0.2, sphere_material));
				}
			}
		}
	}

	auto material1 = make_shared<dielectric>(color(.95, .95, .95), 1.5);
	world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

	auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
	world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

	auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
	world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

	return world;
}

hittable_list two_checker_spheres()
{
	hittable_list objects;
	auto checker = make_shared<checker_texture>(color(.2, .3, .1), color(.9, .9, .9));

	objects.add(make_shared<sphere>(point3(0.0, -10.0, 0.0), 10.0, make_shared<lambertian>(checker)));
	objects.add(make_shared<sphere>(point3(0.0, 10.0, 0.0), 10.0, make_shared<lambertian>(checker)));

	return objects;
}

hittable_list two_perlin_spheres()
{
	hittable_list objects;

	auto perlin_texture = make_shared<noise_texture>(5);
	objects.add(make_shared<sphere>(point3(0.0, -1000.0, 0.0), 1000.0, make_shared<lambertian>(perlin_texture)));
	objects.add(make_shared<sphere>(point3(0.0, 2.0, 0.0), 2.0, make_shared<lambertian>(perlin_texture)));

	return objects;
}

hittable_list earth()
{
	hittable_list objects;
	auto earth_texture = make_shared<image_texture>("earth8k+.jpg", .5);
	auto earth_surface = make_shared<metal>(earth_texture, 1.);
	objects.add(make_shared<sphere>(point3(0.0, 0.0, 0.0), 2.0, earth_surface));

	auto checker = make_shared<checker_texture>(color(.3, .2, .1) / 10., color(.004));
	objects.add(make_shared<xz_rect>(-1000., 1000., -1000., 1000., -2., make_shared<metal>(checker, .5)));

	auto background_light = make_shared<diffuse_light>(color(1.0, .95, .75), .05);
	objects.add(make_shared<sphere>(point3(0., 0., 0.), 20., background_light));

	auto sun_light = make_shared<diffuse_light>(color(1.0, .95, .75), 1.);
	objects.add(make_shared<sphere>(point3(13., .0, -3.) * 3. + point3(.0, 28.0258, .0), 45., sun_light, false));

	/*auto athmosphere = make_shared<sphere>(point3(0.0, 0.0, 0.0), 2.333, earth_surface);
	objects.add(make_shared<constant_medium>(athmosphere, .025, color(.9, .95, 1.)));*/

	return objects;
}

hittable_list simple_light()
{
	hittable_list objects;

	auto perlin_texture = make_shared<noise_texture>(5);
	objects.add(make_shared<xz_rect>(-250, 250, -250, 250, 0.0, make_shared<lambertian>(perlin_texture)));

	auto earth_texture = make_shared<image_texture>("earth8k+.jpg", .5);
	auto earth_surface = make_shared<lambertian>(earth_texture);
	objects.add(make_shared<sphere>(point3(0.0, 2.0, 0.0), 2.0, earth_surface));

	auto difflight_right = make_shared<diffuse_light>(color(.3, .3, 1.0), 5.0);
	auto difflight_left = make_shared<diffuse_light>(color(1.0, .3, .3), 5.0);
	auto difflight_up = make_shared<diffuse_light>(color(.3, 1.0, .3), 3.0);
	auto difflight_back = make_shared<diffuse_light>(color(.91, .38, 0.0), 1.0);
	auto difflight_front = make_shared<diffuse_light>(color(0.0, .72, .92), 1.0);

	objects.add(make_shared<xy_rect>(-1, 1, 1, 3, -3, difflight_right));
	objects.add(make_shared<xy_rect>(-1, 1, 1, 3, 3, difflight_left));
	objects.add(make_shared<sphere>(point3(0.0, 5.0, 0.0), .5, difflight_up));
	objects.add(make_shared<yz_rect>(0.0, 15.0, -15, 15, -25.0, difflight_back));
	objects.add(make_shared<yz_rect>(0.0, 15.0, -15, 15, 35.0, difflight_front));

	return objects;
}

hittable_list cornell_box(bool smoke = false)
{
	hittable_list objects;

	auto red = make_shared<lambertian>(color(.65, .05, .05));
	auto white = make_shared<lambertian>(color(.73, .73, .73));
	auto green = make_shared<lambertian>(color(.12, .45, .15));
	auto light = make_shared<diffuse_light>(color(.95, .95, 1.0), 10.0);

	objects.add(make_shared<yz_rect>(0, 555, 0, 555, 555, green));
	objects.add(make_shared<yz_rect>(0, 555, 0, 555, 0, red));
	objects.add(make_shared<xz_rect>(113, 443, 127, 432, 554, light));
	objects.add(make_shared<xz_rect>(0, 555, 0, 555, 0, white));
	objects.add(make_shared<xz_rect>(0, 555, 0, 555, 555, white));
	objects.add(make_shared<xy_rect>(0, 555, 0, 555, 555, white));

	shared_ptr<hittable> box1 = make_shared<box>(point3(0, 0, 0), point3(165, 330, 165), white);
	box1 = make_shared<rotate_y>(box1, 15);
	box1 = make_shared<translate>(box1, vec3(265, 0, 295));

	shared_ptr<hittable> box2 = make_shared<box>(point3(0, 0, 0), point3(165, 165, 165), white);
	box2 = make_shared<rotate_y>(box2, -18);
	box2 = make_shared<translate>(box2, vec3(130, 0, 65));

	if (!smoke)
	{
		objects.add(box1);
		objects.add(box2);
	}
	else
	{
		objects.add(make_shared<constant_medium>(box1, .01, color(0.0)));
		objects.add(make_shared<constant_medium>(box2, .005, color(1.0)));
	}

	return objects;
}

hittable_list final_scene() {
	hittable_list boxes1;
	auto ground = make_shared<lambertian>(color(0.48, 0.83, 0.53));

	const int boxes_per_side = 10;
	for (int i = 0; i < boxes_per_side; i++) {
		for (int j = 0; j < boxes_per_side; j++) {
			auto w = 2000.0 / boxes_per_side;
			auto x0 = -1000.0 + i * w;
			auto z0 = -1000.0 + j * w;
			auto y0 = 0.0;
			auto x1 = x0 + w;
			auto y1 = random_double(1, 101);
			auto z1 = z0 + w;

			boxes1.add(make_shared<box>(point3(x0, y0, z0), point3(x1, y1, z1), ground));
		}
	}

	hittable_list objects;

	objects.add(make_shared<bvh_node>(boxes1, 0, 1));

	auto light = make_shared<diffuse_light>(color(7, 7, 7));
	objects.add(make_shared<xz_rect>(123, 423, 147, 412, 554, light));

	auto center1 = point3(400, 400, 200);
	auto center2 = center1 + vec3(30, 0, 0);
	auto moving_sphere_material = make_shared<lambertian>(color(0.7, 0.3, 0.1));
	objects.add(make_shared<moving_sphere>(center1, center2, 0, 1, 50, moving_sphere_material));

	objects.add(make_shared<sphere>(point3(260, 150, 45), 50, make_shared<dielectric>(1.5)));
	objects.add(make_shared<sphere>(
		point3(0, 150, 145), 50, make_shared<metal>(color(0.8, 0.8, 0.9), 1.0)
		));

	auto boundary = make_shared<sphere>(point3(360, 150, 145), 70, make_shared<dielectric>(1.5));
	objects.add(boundary);
	objects.add(make_shared<constant_medium>(boundary, 0.2, color(0.2, 0.4, 0.9)));
	boundary = make_shared<sphere>(point3(0, 0, 0), 5000, make_shared<dielectric>(1.5));
	objects.add(make_shared<constant_medium>(boundary, .0001, color(1, 1, 1)));

	auto emat = make_shared<lambertian>(make_shared<image_texture>("earth8k+.jpg"));
	objects.add(make_shared<sphere>(point3(400, 200, 400), 100, emat));
	auto pertext = make_shared<noise_texture>(.1);
	objects.add(make_shared<sphere>(point3(220, 280, 300), 80, make_shared<lambertian>(pertext)));

	hittable_list boxes2;
	auto white = make_shared<lambertian>(color(.73, .73, .73));
	int ns = 1000;
	for (int j = 0; j < ns; j++) {
		boxes2.add(make_shared<sphere>(point3::random(0, 165), 10, white));
	}

	objects.add(make_shared<translate>(
		make_shared<rotate_y>(
			make_shared<bvh_node>(boxes2, 0.0, 1.0), 15),
		vec3(-100, 270, 395)
		)
	);

	return objects;
}

hittable_list hdr_scene()
{
	hittable_list world;

	//world.add(make_shared<rect>(point3(-250., -3., -250.), vec3(500., .0, .0), vec3(.0, .0, 500.), make_shared<lambertian>(make_shared<noise_texture>(5.))));
	world.add(make_shared<xz_rect>(-1000., 1000., -1000., 1000., -3., make_shared<lambertian>(make_shared<noise_texture>(.333))));

	world.add(make_shared<sphere>(point3(.0, .0, 5.), .5, make_shared<dielectric>(color(1.), 1.5)));
	world.add(make_shared<sphere>(point3(0), 3., make_shared<diffuse_light>(color(1., 1., .85), 5.)));
	world.add(make_shared<sphere>(point3(-4., .0, .0), 3., make_shared<lambertian>(color(.255, .412, .882))));
	world.add(make_shared<sphere>(point3(4., .0, .0), 3., make_shared<metal>(color(.196, .804, .196), .5)));

	return world;
}

class task
{
public:
	void operator() (int _id, int num_of_threads, atomic<int>* progress, int width, int height, int samples_per_pixel, camera cam, color background, const hittable& world, int max_depth)
	{
		id = _id;
		buff = new BYTE[3 * ceil(width * height / (float)num_of_threads)];

		for (int i = id, pointer = 0; i < width * height; i += num_of_threads)
		{
			int y = floor(i / (double)width);
			int x = i - y * width;

			color pixel_color(0.0, 0.0, 0.0);
			for (int s = 0; s < samples_per_pixel; ++s)
			{
				auto u = double(x + random_double()) / (width - 1);
				auto v = double(y + random_double()) / (height - 1);
				ray r = cam.get_ray(u, v);
				pixel_color += ray_color(r, background, world, max_depth);
			}

			write_color(buff, pointer * 3, pixel_color, samples_per_pixel); //writing into bitmap buffer

			pointer++;
			progress->fetch_add(1);
		}
	}

public:
	int id;
	BYTE* buff;
};

int main()
{
	//Image
	const double aspect_ratio = 16.0 / 9.0;
	const int image_width = 1920;
	const int image_height = static_cast<int>(image_width / aspect_ratio);
	const int samples_per_pixel = 2560;
	const int max_depth = 32;

	//Render Variables
	const int number_of_threads = thread::hardware_concurrency() <= 0 ? 4 : thread::hardware_concurrency();
	BYTE* image_buffer = new BYTE[3 * image_width * image_height];
	vector<thread> threads;
	vector<task*> tasks;

	atomic<bool> report(true);
	atomic<int> thread_progress(0);

	//Camera
	point3 lookfrom;
	point3 lookat;
	vec3 vup(0.0, 1.0, 0.0);
	color background(0.70, 0.80, 1.00);
	auto vfov = 40.0;
	double aperture = 0.0;
	double dist_to_focus = 10.0;

	//World
	hittable_list world;
	switch (0)
	{
	case 1:
		world = random_scene();
		lookfrom = point3(13.0, 2.0, 3.0);
		lookat = point3(0.0, 0.0, 0.0);
		vfov = 20.0;
		aperture = 0.075;
		dist_to_focus = (lookat - lookfrom).length();
		break;
	case 2:
		world = two_checker_spheres();
		lookfrom = point3(13.0, 2.0, 3.0);
		lookat = point3(0, 0, 0);
		vfov = 20.0;
		dist_to_focus = (lookat - lookfrom).length();
		break;
	case 3:
		world = two_perlin_spheres();
		lookfrom = point3(13.0, 2.0, 3.0);
		lookat = point3(0, 0, 0);
		vfov = 20.0;
		dist_to_focus = (lookat - lookfrom).length();
		break;
	case 4:
		world = earth();
		lookfrom = point3(13.0, 6.666, -3.0);
		lookat = point3(0, 0, 0);
		vfov = 20.0;
		aperture = 0.075;
		dist_to_focus = (lookat - lookfrom).length();
		break;
	case 5:
		world = simple_light();
		background = color(0.0);
		lookfrom = point3(26, 3, 6);
		lookat = point3(0, 2, 0);
		vfov = 20.0;
		dist_to_focus = (lookat - lookfrom).length();
		break;
	case 6:
		world = cornell_box(true);
		background = color(0, 0, 0);
		lookfrom = point3(278, 278, -800);
		lookat = point3(278, 278, 0);
		vfov = 40.0;
		dist_to_focus = (lookat - lookfrom).length();
		break;
	case 7:
		world = final_scene();
		background = color(0, 0, 0);
		lookfrom = point3(478, 278, -600);
		lookat = point3(278, 278, 0);
		vfov = 40.0;
		dist_to_focus = (lookat - lookfrom).length();
		break;
	default:
	case 8:
		world = hdr_scene();
		background = color(.1);
		lookfrom = point3(-10., 0.01, 20.);
		lookat = point3(.0);
		vfov = 30.0;
		aperture = .1;
		dist_to_focus = (lookat - lookfrom).length();
		break;
	}

	camera cam(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0);

	//Render
	auto startTime = high_resolution_clock::now();
	//std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";
	//for (int j = image_height - 1; j >= 0; --j)
	//{
	//	for (int i = 0; i < image_width; ++i)
	//	{
	//		color pixel_color(0.0, 0.0, 0.0);
	//		for (int s = 0; s < samples_per_pixel; ++s)
	//		{
	//			auto u = double(i + random_double()) / (image_width - 1);
	//			auto v = double(j + random_double()) / (image_height - 1);
	//			ray r = cam.get_ray(u, v);
	//			pixel_color += ray_color(r, background, world, max_depth);
	//		}
	//
	//		write_color(std::cout, pixel_color, samples_per_pixel); //writing into the ppm file
	//		write_color(image_buffer, (j * image_width + i) * 3, pixel_color, samples_per_pixel); //writing into bitmap buffer
	//	}
	//	write_status(startTime, image_height - j, image_height);
	//}

	//Starting threads
	thread progress_thread(report_status, &report, startTime, &thread_progress, image_width * image_height);
	for (int i = 0; i < number_of_threads; i++)
	{
		task* t = new task();
		threads.push_back(thread(std::ref(*t), i, number_of_threads, &thread_progress, image_width, image_height, samples_per_pixel, cam, background, world, max_depth));
		tasks.push_back(t);
	}

	//Wait for threads to finish
	for (thread& t : threads)
	{
		if (t.joinable())
		{
			t.join();
		}
	}
	report.store(false);
	write_status(startTime, 1, 1);

	//Constract final image
	for (auto t : tasks)
	{
		for (int i = t->id, pointer = 0; i < image_width * image_height; i += number_of_threads)
		{
			int y = floor(i / (double)image_width);
			int x = i - y * image_width;

			color c = read_color_raw(t->buff, pointer * 3);
			write_color_raw(image_buffer, (y * image_width + x) * 3, c);

			pointer++;
		}
	}

	//Save bitmap
	CHAR mypicturespath[MAX_PATH];
	SHGetFolderPath(NULL, CSIDL_MYPICTURES, NULL, SHGFP_TYPE_CURRENT, mypicturespath);
	string picPath(mypicturespath);

	std::stringstream ss;
	ss << time(0);

	string folderPath = picPath + "\\Renders";
	if (CreateDirectory(folderPath.c_str(), NULL) || ERROR_ALREADY_EXISTS == GetLastError())
	{
		string path = folderPath + "\\" + ss.str() + ".bmp";
		SaveBitmapToFile((BYTE*)image_buffer, image_width, image_height, 24, 0, path.c_str());
	}
	else
	{
		std::cerr << "ERROR: Render could not be saved!";
	}
	delete[] image_buffer;

	std::cerr << "\nRender completed in " << duration_cast<microseconds>(high_resolution_clock::now() - startTime).count() / 1000000.0 << "s.\n";
	Beep(1000, 200);
	Beep(1000, 800);
	std::cerr << "\a";
	system("pause");
}