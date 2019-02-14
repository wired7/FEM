#pragma once
#include <vector>
#include <iostream>
#include "glm.hpp"

using namespace std;
using namespace glm;

namespace ImplicitGeo
{
	class ImplicitGeometry
	{
	public:
		ImplicitGeometry() {};
		~ImplicitGeometry() {};
		virtual bool intersects(vec3 origin, vec3 direction) const = 0;
		virtual float intersection(vec3 origin, vec3 direction) const = 0;
		virtual vec3 getNormal(vec3 point) = 0;
		virtual void setPosition(vec3 pos) = 0;
		virtual void translate(vec3 trans) = 0;
	};

	class Plane : public ImplicitGeometry
	{
	public:
		vec3 point;
		vec3 normal;
		Plane() {};
		Plane(vec3, vec3);
		Plane(vec3, vec3, vec3);
		~Plane() {};
		virtual bool intersects(vec3, vec3) const override;
		virtual float intersection(vec3, vec3) const override;
		vec3 getNormal(vec3);
		void setPosition(vec3) {};
		void translate(vec3) {};
	};

	class Sphere : public ImplicitGeometry
	{
	public:
		vec3 center;
		float radius;
		Sphere(vec3, double);
		~Sphere() {};
		bool intersects(vec3, vec3) const override;
		float intersection(vec3, vec3) const override;
		vec3 getNormal(vec3);
		double getRadius();
		void setPosition(vec3);
		void translate(vec3);
	};

	class Triangle : public Plane
	{
	public:
		vec3 point1;
		vec3 point2;
		vec3 point3;
		Triangle(vec3, vec3, vec3);
		~Triangle() {};
		float area(vec3, vec3, vec3);
		bool intersects(const Triangle&) const;
		bool intersects(vec3, vec3) const override { return false; };
		float intersection(vec3 origin, vec3 direction) const override;
	};
}