#pragma once
#include <string>
#include <memory>
#include <vector>
#include "Color.h"

namespace Redline
{
	class Bitmap2D
	{
	public:

		const unsigned int Width;
		const unsigned int Height;

		char* const Pixels;
		std::vector<char*> MipmapChain;
		unsigned MaxMipLevel;

		Bitmap2D(unsigned int width, unsigned int height);
		~Bitmap2D();

		mathfu::vec3 SampleTexturePoint(const mathfu::vec2& uv);
		mathfu::vec3 SampleTexturePoint(const mathfu::vec2& uv, unsigned mipmapLevel);

		mathfu::vec3 SampleTextureBilinear(const mathfu::vec2& uv);
		mathfu::vec3 SampleTextureBilinear(const mathfu::vec2& uv, unsigned mipmapLevel);

		mathfu::vec3 SampleTextureTrilinear(const mathfu::vec2& uv, float mipmapLevel);

		void WriteColor(unsigned x, unsigned y, const mathfu::vec4& color);

		void GenerateMips();

		static std::shared_ptr<Bitmap2D> LoadFromFile(const std::string& filename);

	private:
		unsigned GenerateMipLevel(unsigned upperWidth, unsigned upperHeight, unsigned currentLevel);
		mathfu::vec4 ReadPixelFromLevel(unsigned levelWidth, unsigned levelHeight, unsigned x, unsigned y, unsigned level);

	};
}


