#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <fstream>
#include <vector>
#include <iostream>
#include <cassert>
#include <random>
#include <algorithm>
#include <Eigen>

using namespace Eigen;

// image background color
Vector3f bgcolor(1.0f, 1.0f, 1.0f);
// lights in the scene
std::vector<Vector3f> lightPositions = { Vector3f(0.0, 140, 60)
									   , Vector3f(-60.0, 60, 60)
									   , Vector3f(60.0, 60, 60 ) }; //creating area light.

class Sphere
{
public:
	Vector3f center;  // position of the sphere
	float radius;  // sphere radius
	Vector3f surfaceColor; // surface color
	
  Sphere(
		const Vector3f &c,
		const float &r,
		const Vector3f &sc) :
		center(c), radius(r), surfaceColor(sc)
	{
	}

    // line vs. sphere intersection (note: this is slightly different from ray vs. sphere intersection!)
	bool intersect(const Vector3f &rayOrigin, const Vector3f &rayDirection, float &t0, float &t1) const
	{
		Vector3f l = center - rayOrigin;
		float tca = l.dot(rayDirection);
		if (tca < 0) return false;
		float d2 = l.dot(l) - tca * tca;
		if (d2 > (radius * radius)) return false;
        float thc = sqrt(radius * radius - d2);
		t0 = tca - thc;
		t1 = tca + thc;

		return true;
	}
};

// diffuse reflection model
Vector3f diffuse(const Vector3f &L, // direction vector from the point on the surface towards a light source
	const Vector3f &N, // normal at this point on the surface
	const Vector3f &diffuseColor,
	const float kd // diffuse reflection constant
	)
{
	Vector3f resColor = Vector3f::Zero();
	resColor = 0.25 * kd * std::max(L.dot(N), 0.0f) * diffuseColor;
	// TODO: implement diffuse shading model

	return resColor;
}

// Phong reflection model
Vector3f phong(const Vector3f &L, // direction vector from the point on the surface towards a light source
               const Vector3f &N, // normal at this point on the surface
               const Vector3f &V, // direction pointing towards the viewer
               const Vector3f &diffuseColor, 
               const Vector3f &specularColor, 
               const float kd, // diffuse reflection constant
               const float ks, // specular reflection constant
               const float alpha) // shininess constant
{
	Vector3f Es;
	Vector3f R;
	Vector3f resColor = Vector3f::Zero();
	Vector3f Ed = diffuse(L, N, diffuseColor, kd);
	//reflection ray
	R = (2 * N) * (std::max(N.dot(L), 0.0f)) - L;
	//Specular lighting
	Es = .25 * specularColor * ks * (pow (std::max(R.dot(V), 0.0f), alpha));
	return Ed + Es;
}
//
Vector3f Lighting(Vector3f lightOrigin, Vector3f lightDirection, const std::vector<Sphere> &spheres, const Sphere &sphere, Vector3f invRayDirection) {
	Vector3f reflection;
	Vector3f rayIntersect;
	Vector3f point;
	Vector3f pixNormal;
	float t0, t1;
	//Loop through spheres to see if the light ray is blocked
	for each(Sphere sphere in spheres) {
		if (sphere.intersect(lightOrigin, lightDirection, t0, t1)) {
			return 0 * sphere.surfaceColor;
		}
	}
	//Normal of the pixIntersection
	pixNormal = lightOrigin - sphere.center;
	pixNormal.normalize();
	//return .333 * sphere.surfaceColor; //Part 2
	//return diffuse(lightDirection, pixNormal, sphere.surfaceColor, 1); //Part 3
	return phong(lightDirection, pixNormal, invRayDirection, sphere.surfaceColor, Vector3f::Ones(), 1, 3, 100); //Part 3
	
	
}
bool findSphere(const Vector3f &rayOrigin, const Vector3f &rayDirection, const std::vector<Sphere> &spheres, Sphere &sphere) {
	float smol;
	float t0, t1;
	int sphereID = -1;
	bool firstIntersect = true;
	//find smallest t0 first thing the ray hits
	for (int i = 0; i < spheres.size(); i++) {
		if (spheres[i].intersect(rayOrigin, rayDirection, t0, t1)) {
			if (firstIntersect) {
				sphereID = i;
				smol = t0;
				firstIntersect = false;
			}
			else if (t0 < smol) {
				sphereID = i;
				smol = t0;
			}
		}
	}
	if (sphereID >= 0) {
		sphere = spheres[sphereID];
		return true;
	}
	else {
		return false;
	}
}
//This is a ray or line.
Vector3f trace(
	const Vector3f &rayOrigin,
	const Vector3f &rayDirection,
	const std::vector<Sphere> &spheres, int depth)
{
	int maxDepth = 2;
	bool isSpecular;
	Sphere sphere(Vector3f::Zero(), 0, Vector3f::Zero());
	Vector3f pixelColor = Vector3f::Zero();
	Vector3f pixIntersection;
	Vector3f lightDirection;
	Vector3f pixNormal;
	//find smallest t0 first thing the ray hits
	if (!findSphere(rayOrigin, rayDirection, spheres, sphere)) {
		pixelColor = bgcolor;
	}
	//intersection found
	else {
		//t1 and t0. A formality
		float t0, t1;
		isSpecular = sphere.intersect(rayOrigin, rayDirection, t0, t1);
		//find the pixel intersection
		pixIntersection = rayOrigin + (t0 * rayDirection);
		//Part 1
		//return Vector3f(1, 0, 0);
		//part 2
		//return sphere.surfaceColor;
		for each (Vector3f light in lightPositions) { //This for each must be commented to replicate part 1.
			//ray from the pixel intersection to the light source
			lightDirection = (light - pixIntersection);
			lightDirection.normalize();
			//phong + diffusion
			pixelColor += Lighting(pixIntersection, lightDirection, spheres, sphere, -rayDirection);
		}
		depth += 1;
		if (depth < maxDepth) {
			Vector3f N, L, R;
			N = (pixIntersection - sphere.center);
			N.normalize();
			if (isSpecular) {
				R = (2 * N) * (std::max(N.dot(lightDirection), 0.0f)) - lightDirection;
				pixelColor += .333 * trace(rayOrigin, rayDirection, spheres, depth);
			}
		}
	}
	return pixelColor;
}

void render(const std::vector<Sphere> &spheres)
{
  
	int depth = 0; //Added for reflected rays
  unsigned width = 640;
  unsigned height = 480;
  Vector3f *image = new Vector3f[width * height];
  Vector3f *pixel = image;
  float invWidth  = 1 / float(width);
  float invHeight = 1 / float(height);
  float fov = 30;
  float aspectratio = width / float(height);
	float angle = tan(M_PI * 0.5f * fov / 180.f);
	
	// Trace rays
	for (unsigned y = 0; y < height; ++y) 
	{
		for (unsigned x = 0; x < width; ++x) 
		{
			float rayX = (2 * ((x + 0.5f) * invWidth) - 1) * angle * aspectratio;
			float rayY = (1 - 2 * ((y + 0.5f) * invHeight)) * angle;
			Vector3f rayDirection(rayX, rayY, -1);
			rayDirection.normalize();
			*(pixel++) = trace(Vector3f::Zero(), rayDirection, spheres, depth);
		}
	}
	
	// Save result to a PPM image
	std::ofstream ofs("./render.ppm", std::ios::out | std::ios::binary);
	ofs << "P6\n" << width << " " << height << "\n255\n";
	for (unsigned i = 0; i < width * height; ++i) 
	{
		const float x = image[i](0);
		const float y = image[i](1);
		const float z = image[i](2);

		ofs << (unsigned char)(std::min(float(1), x) * 255) 
			  << (unsigned char)(std::min(float(1), y) * 255) 
			  << (unsigned char)(std::min(float(1), z) * 255);
	}
	
	ofs.close();
	delete[] image;
}

int main(int argc, char **argv)
{
	std::vector<Sphere> spheres;
	// position, radius, surface color
	spheres.push_back(Sphere(Vector3f(0.0, -10004, -20), 10000, Vector3f(0.50, 0.50, 0.50)));
	spheres.push_back(Sphere(Vector3f(0.0, 0, -20), 4, Vector3f(1.00, 0.32, 0.36)));
	spheres.push_back(Sphere(Vector3f(5.0, -1, -15), 2, Vector3f(0.90, 0.76, 0.46)));
	spheres.push_back(Sphere(Vector3f(5.0, 0, -25), 3, Vector3f(.65, .77, 0.99)));
	spheres.push_back(Sphere(Vector3f(-5.5, 0, -13), 3, Vector3f(.9, .9, .9)));

	render(spheres);

	return 0;
}
