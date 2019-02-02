#include "stdafx.h"
#include "SphereSurface.h"
#include "../../../Math/Ray.h"
#include "../RayHitSurfaceDetails.h"
#include "../../../Math/Math.h"

using namespace Redline;
using namespace mathfu;

Redline::SphereSurface::SphereSurface(const mathfu::vec3& position, float radius, std::shared_ptr<Material>& material)
{
	_position = position;
	_radius = radius;
	_radiusSquared = radius * radius;

	//Copy material so that the scene can be changed while we render
	_surfaceMaterial = std::make_shared<Material>();
	if(_surfaceMaterial != nullptr)
	{
		memcpy(_surfaceMaterial.get(), material.get(), sizeof(Material));
	}
}

SphereSurface::SphereSurface(const mathfu::vec3& position, float radius)
{
	_position = position;
	_radius = radius;
	_surfaceMaterial = nullptr;
}

bool SphereSurface::AttemptRayIntersection(const Ray& ray, float& outIntersectionDistance, RayHitSurfaceDetails& outSurfaceDetails)
{
	mathfu::vec3 toCenter = _position - ray.Origin;

	if(toCenter.LengthSquared() < _radiusSquared)
	{
		RayHitSurfaceDetails surface;
		const auto normal = (ray.Origin - _position).Normalized();
		surface.SurfaceNormal_Geometry = normal;
		surface.SurfaceNormal = normal;

		vec3 tangent = -vec3::CrossProduct(vec3(0.0f, 1.0f, 0.0f), normal).Normalized();
		vec3 biTangent = vec3::CrossProduct(normal, tangent).Normalized();


		surface.TangentMatrix.GetColumn(0) = tangent.Normalized();
		surface.TangentMatrix.GetColumn(1) = biTangent.Normalized();
		surface.TangentMatrix.GetColumn(2) = normal.Normalized();

	//	surface.TangentMatrix = surface.TangentMatrix.Transpose();

		float r = atan2f(-normal.z, normal.x);

		vec2 uv = vec2(
			(0.5f + r / (2.0f * M_PIF)),
			0.5f - asinf(normal.y) / M_PIF);

		//TODO:Default material if none is set
		_surfaceMaterial->ApplyToSurfaceHit(surface, uv, 0.0f);

		//TODO:What to do about surface when inside sphere, consider returning backface instead, will be important rendering this as a volume
		outIntersectionDistance = 0.0f;
		outSurfaceDetails = surface;
		return true;
	}

	float dot = vec3::DotProduct(toCenter, ray.Direction);

	if(dot < 0.0f)
	{
		return false;
	}

	vec3 projected = ray.Origin + ray.Direction * dot;

	float projectedToCenterDistance = vec3::DistanceSquared(projected, _position);

	if(projectedToCenterDistance > _radiusSquared)
	{
		return false;
	}

	float offset = sqrtf((_radiusSquared) - (projectedToCenterDistance*projectedToCenterDistance));

	float intersectionTime = dot - offset;

	outIntersectionDistance = intersectionTime;

	vec3 intersectionPosition = ray.Origin + ray.Direction * outIntersectionDistance;

	RayHitSurfaceDetails surface;
	const auto normal = (intersectionPosition - _position).Normalized();
	surface.SurfaceNormal = normal;

	float r = atan2f(-normal.z, normal.x);

	vec2 uv = vec2(
		0.5f + r / (2.0f * M_PIF),
		0.5f - asinf(normal.y) / M_PIF);

	//Compute binormal and bitangent

	vec3 tangent = vec3::CrossProduct(vec3(0.0f,1.0f,0.0f), normal).Normalized();
	vec3 biTangent = vec3::CrossProduct(normal, tangent).Normalized();


	surface.TangentMatrix.GetColumn(0) = tangent.Normalized();
	surface.TangentMatrix.GetColumn(1) = biTangent.Normalized();
	surface.TangentMatrix.GetColumn(2) = normal.Normalized();

	//surface.TangentMatrix = surface.TangentMatrix.Transpose();
	//surface.TangentMatrix = surface.TangentMatrix.Inverse();

	//TODO:Default material if none is set
	_surfaceMaterial->ApplyToSurfaceHit(surface, uv, 0.0f);

	outSurfaceDetails = surface;

	return true;
}

bool SphereSurface::CheckRayOcclusion(const Ray& ray, float maxDistance)
{
	vec3 toCenter = _position - ray.Origin;

	//If we are inside the sphere, the ray is definitly occluded
	if (toCenter.Length() < _radius)
	{
		return true;
	}

	float dot = vec3::DotProduct(toCenter, ray.Direction);

	if (dot < 0.0f)
	{
		return false;
	}

	vec3 projected = ray.Origin + ray.Direction * dot;

	float projectedToCenterDistance = vec3::Distance(projected, _position);

	if (projectedToCenterDistance > _radius)
	{
		return false;
	}

	float offset = sqrtf((_radiusSquared)-(projectedToCenterDistance*projectedToCenterDistance));
	float intersectionTime = dot - offset;

	if(intersectionTime <= maxDistance)
	{
		return true;
	}

	return false;
}

BoundingBox SphereSurface::GetBounds()
{
	BoundingBox bounds;

	bounds.Min = vec3(-_radius, -_radius, -_radius) + _position;
	bounds.Max = vec3(_radius, _radius, _radius) + _position;

	return bounds;
}
