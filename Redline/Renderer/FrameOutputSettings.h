#pragma once

namespace Redline
{
	/*
	 * Settings structure used by the renderer to draw a frame
	 */
	struct FrameOutputSettings
	{
		unsigned int OutputWidth;
		unsigned int OutputHeight;
		unsigned int SamplesPerPixel;
	};

	struct FrameQuailtySettings
	{
		bool EnableShadows = true;
		bool EnableSoftShadows = true;
		unsigned AnalyticLightSamples = 2;

		unsigned MaxSpecularBounces = 6;
		unsigned SpecularReflectionSamples = 8;

		unsigned MaxDiffuseBounces = 2;
		unsigned DiffuseReflectionSamples = 8;

	};
}