#pragma once
#include <mathfu/glsl_mappings.h>
#include "Math/ImageRectangle.h"

namespace Redline
{
	class Bitmap2D;

	class Bitmap2DFloat
	{
	public:
		const unsigned int Width;
		const unsigned int Height;

		Bitmap2DFloat(unsigned int width, unsigned int height);
		~Bitmap2DFloat();

		mathfu::vec4 ReadPixel(unsigned x,unsigned y) const;

		float ReadVariance(unsigned x, unsigned y) const;

		void WriteColorSample(unsigned x, unsigned y, const mathfu::vec4& colorSample);

		void MapRegionToBitmap(std::shared_ptr<Bitmap2D>& bitmap,const ImageRectangle& region) const;
	private:
		mathfu::vec4* const _colorBuffer;
		float* const _varianceBuffer;
		unsigned* const _sampleCount;
	};
}
