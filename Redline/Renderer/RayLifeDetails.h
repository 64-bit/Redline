#pragma once
#include "FrameOutputSettings.h"

namespace Redline
{
	//The details of the life of a ray. Used to count bounces and mipmap level reductions
	struct RayLifeDetails
	{
		RayLifeDetails();
		RayLifeDetails(const FrameQuailtySettings& quailtySettings);

		int RemainingSpecularBounces;
		int RemainingDiffuseBounces;

		float MipLevelReduction;
	};

	inline RayLifeDetails::RayLifeDetails(const FrameQuailtySettings& quailtySettings)
	{
		RemainingSpecularBounces = quailtySettings.MaxSpecularBounces;
		RemainingDiffuseBounces = quailtySettings.MaxDiffuseBounces;
		MipLevelReduction = 0.0f;
	}
}
