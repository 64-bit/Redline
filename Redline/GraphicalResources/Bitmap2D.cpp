#include "stdafx.h"
#include "Bitmap2D.h"
#include "../SingleFileLibs/lodepng.h"
#include <iostream>
#include "Color.h"

using namespace Redline;
using namespace mathfu;

Bitmap2D::Bitmap2D(unsigned width, unsigned height) 
: Width(width), Height(height), Pixels(new char[width*height*4])
{
	MipmapChain.push_back(Pixels);
}

Bitmap2D::~Bitmap2D()
{
	delete[] Pixels;
}

mathfu::vec3 Redline::Bitmap2D::SampleTexturePoint(const mathfu::vec2& uv)
{
	return SampleTexturePoint(uv, 0);
}

vec3 Bitmap2D::SampleTexturePoint(const mathfu::vec2 & uv, unsigned mipmapLevel)
{
	mipmapLevel = Clamp(mipmapLevel, (unsigned)0, (unsigned)MipmapChain.size() - 1);

	float levelWidth = Width / pow(2.0f, (float)mipmapLevel);
	float levelHeight = Height / pow(2.0f, (float)mipmapLevel);


	unsigned x = (unsigned)(uv.x * levelWidth);
	x = x % (unsigned)levelWidth;

	unsigned y = (unsigned)(uv.y * levelHeight);
	y = y % (unsigned)levelHeight;

	Color* address = (Color*)MipmapChain[mipmapLevel];

	address += (x + (y*(unsigned)levelWidth));

	Color col = *address;
	return col.ToVec4().xyz();
}

vec3 Bitmap2D::SampleTextureBilinear(const mathfu::vec2& uv)
{
	return SampleTextureBilinear(uv, 0);
}

vec3 Bitmap2D::SampleTextureBilinear(const mathfu::vec2 & uv, unsigned mipmapLevel)
{
	mipmapLevel = Clamp(mipmapLevel, (unsigned)0, (unsigned)MipmapChain.size() - 1);

	float levelWidth = (float)(Width / (1 << mipmapLevel));
	float levelHeight = (float)(Height / (1 << mipmapLevel));

	auto pixelPos = vec2(levelWidth, levelHeight) * uv;

	pixelPos.x = abs(pixelPos.x);
	pixelPos.y = abs(pixelPos.y);

	float posX = pixelPos.x - floorf(pixelPos.x);
	float posY = pixelPos.y - floorf(pixelPos.y);

	Color* colors = (Color*) MipmapChain[mipmapLevel];

	int xLow = (int)floorf(pixelPos.x);
	xLow = xLow % (int)levelWidth;
	int xHigh = (int)ceilf(pixelPos.x);
	xHigh = xHigh % (int)levelWidth;

	int yLow = (int)floorf(pixelPos.y);
	yLow = yLow % (int)levelHeight;
	int yHigh = (int)ceilf(pixelPos.y);
	yHigh = yHigh % (int)levelHeight;


	auto col0 = colors[xLow + (yLow * (unsigned)levelWidth)].ToVec4().xyz();
	auto col1 = colors[xHigh + (yLow * (unsigned)levelWidth)].ToVec4().xyz();
	auto col2 = colors[xLow + (yHigh * (unsigned)levelWidth)].ToVec4().xyz();
	auto col3 = colors[xHigh + (yHigh * (unsigned)levelWidth)].ToVec4().xyz();

	auto colTop = vec3::Lerp(col0, col1, posX);
	auto colBottom = vec3::Lerp(col2, col3, posX);

	return vec3::Lerp(colTop, colBottom, posY);
}

vec3 Bitmap2D::SampleTextureTrilinear(const mathfu::vec2 & uv, float mipmapLevel)
{
	unsigned upper = (unsigned)floorf(mipmapLevel);
	unsigned lower = (unsigned)ceilf(mipmapLevel);

	upper = Clamp(upper, (unsigned)0, (unsigned)MipmapChain.size() - 1);
	lower = Clamp(lower, (unsigned)0, (unsigned)MipmapChain.size() - 1);

	if(upper == lower)
	{
		return SampleTextureBilinear(uv, upper);
	}
	else
	{
		float factor = fmodf(mipmapLevel, 1.0f);
		vec3 upperColor = SampleTextureBilinear(uv, upper);
		vec3 lowerColor = SampleTextureBilinear(uv, lower);
		return vec3::Lerp(upperColor, lowerColor, factor);
	}
}

void Bitmap2D::WriteColor(unsigned x, unsigned y, const vec4& color)
{
	Color asColorType(color);
	Color* asColorArray = reinterpret_cast<Color*>(Pixels);
	asColorArray[x + (y*Width)] = asColorType;
}


void Bitmap2D::GenerateMips()
{
	if(MipmapChain.size() > 1)
	{
		return;//Already generated
	}
	GenerateMipLevel(Width, Height, 1);
}

std::shared_ptr<Bitmap2D> Redline::Bitmap2D::LoadFromFile(const std::string & filename)
{
	std::vector<unsigned char> image;
	unsigned width, height;

	unsigned error = lodepng::decode(image, width, height, filename);
	if (error) std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;

	auto bitmap = std::make_shared<Bitmap2D>(width, height);

	memcpy(bitmap->Pixels, image.data(), image.size());

	bitmap->GenerateMips();

	return bitmap;
}

vec4 Bitmap2D::ReadPixelFromLevel(unsigned levelWidth, unsigned levelHeight, unsigned x, unsigned y, unsigned level)
{
	Color* base = (Color*)MipmapChain[level];
	return base[x + (y*levelWidth)].ToVec4();
}

unsigned Redline::Bitmap2D::GenerateMipLevel(unsigned upperWidth, unsigned upperHeight, unsigned currentLevel)
{
	//This means we have reached the bottom of the chain, return the current level back up
	if (upperWidth == 1
		|| upperHeight == 1)
	{
		return currentLevel;
	}
	//This means we are no longer power of 2, return current level
	if (upperWidth % 2 != 0
		|| upperHeight %2 != 0)
	{
		return currentLevel;
	}

	unsigned width = upperWidth / 2;
	unsigned height = upperHeight / 2;

	char* mipmapLevelPixels = new char[width * height * 4];
	MipmapChain.push_back(mipmapLevelPixels);

	Color* mipmapColors = (Color*)mipmapLevelPixels;

	for(unsigned y = 0; y < height;y++)
	{
		for (unsigned x = 0; x < width; x++)
		{
			vec4 result = vec4(0.0f, 0.0f, 0.0f, 0.0f);
			result += ReadPixelFromLevel(upperWidth, upperHeight, x * 2 + 0, y * 2 + 0,currentLevel-1);
			result += ReadPixelFromLevel(upperWidth, upperHeight, x * 2 + 0, y * 2 + 1, currentLevel - 1);
			result += ReadPixelFromLevel(upperWidth, upperHeight, x * 2 + 1, y * 2 + 0, currentLevel - 1);
			result += ReadPixelFromLevel(upperWidth, upperHeight, x * 2 + 1, y * 2 + 1, currentLevel - 1);
			result /= 4.0f;

			mipmapColors[x + (y*width)] = Color(result);
		}
	}

	return GenerateMipLevel(width,height,currentLevel+1);
}
