#include "stdafx.h"
#include "Bitmap2DFloat.h"
#include <Math/Math.h>
#include "Bitmap2D.h"
#include <cmath>

using namespace Redline;
using namespace mathfu;
using namespace std;

#define BITMAP_INDEX(__x,__y) (__x + __y * Width)

Bitmap2DFloat::Bitmap2DFloat(const unsigned width, const unsigned height)
	: Width(width), Height(height),
	_colorBuffer(new vec4[width*height ]),
	_varianceBuffer(new float[width*height]),
	_sampleCount(new unsigned[width*height])
{
	//Init all colors to black

	for(unsigned i  =0; i < Width*Height;i++)
	{
		_colorBuffer[i] = vec4(0.0f, 0.0f, 0.0f, 0.0f);
		_varianceBuffer[i] = 0.0f;
		_sampleCount[i] = 0;
	}
}

Bitmap2DFloat::~Bitmap2DFloat()
{
	delete _colorBuffer;
	delete _varianceBuffer;
	delete _sampleCount;
}

vec4 Bitmap2DFloat::ReadPixel(const unsigned x, const unsigned y) const
{
	const vec4 rawHDR = _colorBuffer[BITMAP_INDEX(x, y)];
	const unsigned samples = _sampleCount[BITMAP_INDEX(x, y)];
	return rawHDR / static_cast<float>(samples);
}

float Bitmap2DFloat::ReadVariance(const unsigned x, const unsigned y) const
{
	const vec4 rawHDR = _colorBuffer[BITMAP_INDEX(x, y)];
	const float rawLuminanceSquared = _varianceBuffer[BITMAP_INDEX(x, y)];
	const auto samples = static_cast<float>(_sampleCount[BITMAP_INDEX(x, y)]);

	const vec4 averageHDR = rawHDR / samples;
	const float averageLuminanceSquared = rawLuminanceSquared / samples;
	const float averageLuminance = (averageHDR.x + averageHDR.y + averageHDR.z) / 3.0f;

	const float variance = averageLuminanceSquared - (averageLuminance * averageLuminance);
	return variance;
}

void Bitmap2DFloat::WriteColorSample(const unsigned x, const unsigned y, const vec4& colorSample)
{
	for(int i = 0; i <3; i++)
	{
		if(std::isnan(colorSample[i])	//TODO:Figure out where the bad samples come from
			|| std::isinf(colorSample[i]))//Reject terribly broken samples
		{
	
			return;
		}
	}

	_colorBuffer[BITMAP_INDEX(x, y)] += colorSample;
	float luminance = CalculateLuminance(colorSample);
	_varianceBuffer[BITMAP_INDEX(x, y)] += luminance * luminance;
	_sampleCount[BITMAP_INDEX(x, y)] += 1;
}

void Bitmap2DFloat::MapRegionToBitmap(shared_ptr<Bitmap2D>& bitmap, const ImageRectangle& region) const
{
	//See if we need to upscale outselves to the output bitmap
	unsigned upscaleX = 1;
	unsigned upscaleY = 1;

	if(bitmap->Width != Width)
	{
		upscaleX = bitmap->Width / Width;
	}

	if (bitmap->Height != Height)
	{
		upscaleY = bitmap->Height / Height;
	}

	bool isScaling = upscaleX > 1 || upscaleY > 1;

	const auto xEnd = region.X + region.Width;
	const auto yEnd = region.Y + region.Height;

	if(isScaling)
	{
		for (unsigned x = region.X; x < xEnd; x++)
		{
			for (unsigned y = region.Y; y < yEnd; y++)
			{
				auto colorVal = ReadPixel(x, y);
				colorVal.w = 1.0f;

				//upscale move
				for(unsigned xS = 0; xS < upscaleX;xS++)
				{
					for (unsigned yS = 0; yS < upscaleY; yS++)
					{
						int scaledXAddress = x * upscaleX + xS;
						int scaledYAddress = y * upscaleY + yS;
						bitmap->WriteColor(scaledXAddress, scaledYAddress, colorVal);
					}
				}
			}
		}
	}
	else
	{
		for (unsigned x = region.X; x < xEnd; x++)
		{
			for (unsigned y = region.Y; y < yEnd; y++)
			{
				auto colorVal = ReadPixel(x, y);
				colorVal.w = 1.0f;
				bitmap->WriteColor(x, y, colorVal);
			}
		}
	}


}
