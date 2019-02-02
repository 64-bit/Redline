#pragma once
#include "../BRDF.h"

namespace Redline
{
	//A basic implementation of the phong BRDF.
	//This is only included so that we can have a simple and fast render,
	//So we use a bastardisation of the cook-torrence physicality based material properites in order to do our rendering
	class PhongBRDF : public BRDF
	{
	public:
		PhongBRDF(const FrameOutputSettings& frameOuputSettings, const FrameQuailtySettings& frameQuailtySettings);


	private:

		mathfu::vec3 GetColorFromRay(const Ray& ray, unsigned remainingReflectionBounces);

		mathfu::vec3 ShadeSurface(const Ray& viewRay, const RayHitSurfaceDetails& surface, unsigned remainingReflectionBounces);
	};
}