#pragma once
#include <string>
namespace Redline
{
	//A Temporary representation of the GLTF alpha mode
	//As we are doing more realstic rendering than in a game, we will have to have a complicated way to define
	//the absorption material, instead of just a alpha blending color
	//This may be useful for short term rendering of GLTF scenes
	enum class EAlphaMode
	{
		OPAQUE,
		MASK,
		BLEND,
	};

	class MaterialEnums
	{
	public:
		static EAlphaMode ParseEAlphaMode(const std::string& string)
		{
			if (string == "OPAQUE")
			{
				return EAlphaMode::OPAQUE;
			}
			else if(string == "MASK")
			{
				return EAlphaMode::MASK;
			}
			else if(string == "BLEND")
			{
				return EAlphaMode::BLEND;
			}
			else
			{
				return EAlphaMode::OPAQUE;
			}
		}
	};
}