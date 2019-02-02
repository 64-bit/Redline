#include "stdafx.h"
#include "GLTFSceneLoader.h"
#include "tiny_gltf.h"
#include "../GraphicalResources/Material.h"
#include "../GraphicalResources/TriangleMesh.h"
#include "../Scene/SceneObject.h"
#include "../Scene/Scene.h"

#include "../Scene/Components/Renderable/MeshRenderer.h"
#include "Scene/Components/CameraComponent.h"
#include "Scene/Components/Lights/PointLightComponent.h"
#include "Scene/Components/Lights/SpotLightComponent.h"
#include "Scene/Components/Lights/DirectionalLightComponent.h"

using namespace std;
using namespace mathfu;
using namespace Redline;

const float ROUGHNESS_MIN_VALUE = 0.03f;

inline vec4 vec4FromColor(tinygltf::ColorValue color)
{
	return vec4(
		static_cast<float>(color[0]),
		static_cast<float>(color[1]), 
		static_cast<float>(color[2]), 
		static_cast<float>(color[3]));
}

shared_ptr<Scene> GLTFSceneLoader::LoadSceneFromGLTFFile(const string& filename)
{
	printf("Loading scene from file: %s\n", filename.c_str());

	shared_ptr<Scene> sceneResult = make_shared<Scene>("Scene name");
	

	tinygltf::Model model;
	tinygltf::TinyGLTF loader;
	std::string err;
	std::string warn;

	bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, filename);

	if (!err.empty()) {
		printf("Err: %s\n", err.c_str());
	}

	if (!warn.empty()) {
		printf("Warn: %s\n", warn.c_str());
	}

	for(auto& val : model.nodes)
	{
		printf("Node name: %s\n", val.name.c_str());
	}


	//The process for loading a scene will go as follows

	//Load all images into internal bitmap image format
	LoadImages(model, Images);
	//Load all materials into internal material format
	LoadMaterials(model, Images, Materials);
	//Load all meshes into internal mesh format
	LoadMeshes(model, Meshes);
	//Create scene objects and add mesh renderer components to them.
	FillScene(model, sceneResult);

	return sceneResult;
}

void GLTFSceneLoader::LoadImages(tinygltf::Model& gltfFile, vector<shared_ptr<Bitmap2D>>& imagesOut)
{
	for(auto& gltfImage : gltfFile.images)
	{
		auto newImage = make_shared<Bitmap2D>(gltfImage.width, gltfImage.height);

		if(gltfImage.component == 4)
		{
			//no alterations needed, copy data into image
			memcpy(newImage->Pixels, gltfImage.image.data(), gltfImage.image.size());
		}
		else if(gltfImage.component == 3)
		{
			//pad image with 0 value alpha values
			unsigned pixelCount = gltfImage.width * gltfImage.height;

			for(unsigned i = 0; i < pixelCount;i++)
			{
				newImage->Pixels[i * 4 + 0] = gltfImage.image[i * 3 + 0];
				newImage->Pixels[i * 4 + 1] = gltfImage.image[i * 3 + 1];
				newImage->Pixels[i * 4 + 2] = gltfImage.image[i * 3 + 2];
				newImage->Pixels[i * 4 + 3] = 0;
			}
		}
		else
		{
			//Cannot load this image
			printf("Cannot load gltf image %s, invalid component count: %d\n", gltfImage.name.c_str(), gltfImage.component);
			continue;
		}
		imagesOut.push_back(newImage);
	}
}

void GLTFSceneLoader::LoadMaterials(
	tinygltf::Model& gltfFile,
	const vector<shared_ptr<Bitmap2D>>& images,
	vector<shared_ptr<Material>>& materialsOut)
{
	for(auto& gltfMaterial : gltfFile.materials)
	{
		auto newMaterial = make_shared<Material>();

		auto& pbrParams = gltfMaterial.values;
		auto& additionalParams = gltfMaterial.additionalValues;

		//PBR Paramaters
		newMaterial->Albedo = TryGetMaterialVector(pbrParams, "baseColorFactor", vec4(0.9f, 0.9f, 0.9f, 1.0f)).xyz();
		newMaterial->Metalness = TryGetMaterialFloat(pbrParams, "metallicFactor", 1.0f);
		newMaterial->Roughness = TryGetMaterialFloat(pbrParams, "roughnessFactor", 1.0f);

		if(newMaterial->Roughness < ROUGHNESS_MIN_VALUE)
		{
			newMaterial->Roughness = ROUGHNESS_MIN_VALUE;
		}

		newMaterial->AlbedoTexture = TryGetTexture(pbrParams, gltfFile.textures, images, "baseColorTexture");
		newMaterial->MetalnessRoughnessTexture = TryGetTexture(pbrParams, gltfFile.textures, images, "metallicRoughnessTexture");

		//Generic surface paramaters
		newMaterial->Emission = TryGetMaterialVector(additionalParams, "emissiveFactor", vec4(0.0f,0.0f,0.0f,0.0f)).xyz();

		//TODO:Read normal map settings as per https://github.com/KhronosGroup/glTF/blob/master/specification/2.0/schema/material.normalTextureInfo.schema.json
		newMaterial->NormalMap = TryGetTexture(additionalParams, gltfFile.textures, images, "normalTexture");
		//TODO:Read occlusion settings as per https://github.com/KhronosGroup/glTF/blob/master/specification/2.0/schema/material.occlusionTextureInfo.schema.json
		newMaterial->AmbientOcclusionMap = TryGetTexture(additionalParams, gltfFile.textures, images, "occlusionTexture");
		newMaterial->EmissionMap = TryGetTexture(additionalParams, gltfFile.textures, images, "emissiveTexture");
		
		newMaterial->AlphaMode = MaterialEnums::ParseEAlphaMode(
			TryGetMaterialString(additionalParams,"alphaMode","OPAQUE"));
		newMaterial->AlphaCutoff = TryGetMaterialFloat(additionalParams, "alphaCutoff", 0.5f);

		newMaterial->IsDoubleSided = TryGetMaterialBool(additionalParams, "doubleSided", true);

		materialsOut.push_back(newMaterial);
	}
}

void GLTFSceneLoader::LoadMeshes(tinygltf::Model& gltfFile, vector<shared_ptr<TriangleMesh>>& meshesOut)
{	
	for(auto& mesh : gltfFile.meshes)
	{
		auto newMesh = std::make_shared<TriangleMesh>(mesh.name, mesh, gltfFile);
		meshesOut.push_back(newMesh);
	}
}

void GLTFSceneLoader::FillScene(tinygltf::Model& gltfFile, std::shared_ptr<Scene>& scene)
{
	auto& gltfScene = gltfFile.scenes[0];
	for(auto rootNode : gltfScene.nodes)
	{
		CreateNode(gltfFile,gltfFile.nodes[rootNode], nullptr, scene);
	}
}

void GLTFSceneLoader::CreateNode(tinygltf::Model& gltfFile, tinygltf::Node& gltfNode, shared_ptr<SceneObject> parent, shared_ptr<Scene>& scene)
{
	auto sceneObject = scene->CreateSceneObject(gltfNode.name);
	if(parent != nullptr)
	{
		sceneObject->Transform.SetParent(&parent->Transform);
	}

	//Read transforms

	if(gltfNode.translation.size() == 3)
	{
		vec3 translation; for (int i = 0; i < 3; i++) { translation[i] = (float)gltfNode.translation[i]; }
		sceneObject->Transform.SetLocalPosition(translation);
	}

	if (gltfNode.rotation.size() == 4)
	{
		quat rotation;
		rotation[0] = static_cast<float>(gltfNode.rotation[3]);
		rotation[1] = static_cast<float>(gltfNode.rotation[0]);
		rotation[2] = static_cast<float>(gltfNode.rotation[1]);
		rotation[3] = static_cast<float>(gltfNode.rotation[2]);
		
		sceneObject->Transform.SetLocalRotation(rotation);
	}

	if (gltfNode.scale.size() == 3)
	{
		vec3 scale; for (int i = 0; i < 3; i++) { scale[i] = (float)gltfNode.scale[i]; }
		sceneObject->Transform.SetLocalScale(scale);
	}

	if (gltfNode.matrix.size() == 16)
	{
		mat4 tx;
		for(int i = 0; i < 16;i++)
		{
			tx[i] = static_cast<float>(gltfNode.matrix[i]);
		}

		vec3 translation = tx.TranslationVector3D();
		quat rotation = quat::FromMatrix(tx.ToRotationMatrix(tx));
		sceneObject->Transform.SetLocalPosition(translation);
		sceneObject->Transform.SetLocalRotation(rotation);
	}

	//Load meshes
	if(gltfNode.mesh >= 0)
	{
		//Append mesh
		auto meshRenderer = sceneObject->AddComponent<MeshRenderer>();
		meshRenderer->Mesh = Meshes[gltfNode.mesh];//TODO:Handle default matieral


		for(auto& primitive : gltfFile.meshes[gltfNode.mesh].primitives)
		{
			if(primitive.material >=0)
			{
				meshRenderer->SurfaceMaterials.push_back(Materials[primitive.material]);
			}
			else
			{
				meshRenderer->SurfaceMaterials.push_back(Materials[0]);//TODO:Default materials
			}
		}

	}

	//Load camera
	if(gltfNode.camera >= 0)
	{
		auto& camera = gltfFile.cameras[gltfNode.camera];

		auto cameraComponent = sceneObject->AddComponent<CameraComponent>();
		cameraComponent->AspectRatio = camera.perspective.aspectRatio;
		cameraComponent->YAxisFieldofViewRadians = camera.perspective.yfov;
	}

	//Load Lights
	//TODO:Hack in addition export values in the blender exporter
	const string lightKey = "KHR_lights";
	auto mainKHRLightsItr = gltfFile.extensions.find(lightKey);
	if (mainKHRLightsItr != gltfFile.extensions.end())
	{

		auto& lightsArray = mainKHRLightsItr->second.Get("lights");

		//Try get extension for lights
		auto khrLights = gltfNode.extensions.find(lightKey);
		if (khrLights != gltfNode.extensions.end())
		{
			auto lightIndex = (int)khrLights->second.Get("light").Get<int>();

			auto& lightObject = lightsArray.Get(lightIndex);

			auto& type = lightObject.Get("type").Get<string>();

			float power = (float)lightObject.Get("intensity").Get<double>();
			auto& colorArray = lightObject.Get("color");
			vec3 lightColor;
			for(int i = 0; i < 3;i++)
			{
				lightColor[i] = (float)colorArray.Get(i).Get<double>();
			}
			//TODO: Fix handling of power and radius
			if(type == "point")
			{
				auto pointLightComponent = sceneObject->AddComponent<PointLightComponent>();
				pointLightComponent->LightSettings.Color = lightColor;
				pointLightComponent->LightSettings.Power = power * (100.0f / (float)M_PI);
				pointLightComponent->LightSettings.LightRadius = power + power * power;
			}
			else if(type == "spot")
			{
				auto spotLightComponent = sceneObject->AddComponent<SpotLightComponent>();
				spotLightComponent->LightSettings.Color = lightColor;
				spotLightComponent->LightSettings.Power = power * (100.0f / (float)M_PI);
				spotLightComponent->LightSettings.LightRadius = 10 + power + power * power;

				auto& spotObject = lightObject.Get("spot");
				spotLightComponent->LightSettings.InnerAngleCos = (float)cos(spotObject.Get("innerConeAngle").Get<double>());
				spotLightComponent->LightSettings.OuterAngleCos = (float)cos(spotObject.Get("outerConeAngle").Get<double>());
			}
			else if(type == "directional")
			{
				auto directionalLightComponent = sceneObject->AddComponent<DirectionalLightComponent>();
				directionalLightComponent->LightSettings.Color = lightColor;
				directionalLightComponent->LightSettings.Power = power;
				directionalLightComponent->LightSettings.Direction = sceneObject->Transform.GetRotation() * vec3(0.0f, 0.0f, -1.0f);
			}
		}
	}

	for(int childNode : gltfNode.children)
	{
		CreateNode(gltfFile, gltfFile.nodes[childNode], sceneObject, scene);
	}
}

float GLTFSceneLoader::TryGetMaterialFloat(tinygltf::ParameterMap& paramaters, const char* paramaterName, float defaultValue)
{
	if (paramaters.find(paramaterName) != paramaters.end())
	{
		return static_cast<float>(paramaters[paramaterName].Factor());
	}
	return defaultValue;
}

vec4 GLTFSceneLoader::TryGetMaterialVector(tinygltf::ParameterMap& paramaters, const char* paramaterName, const vec4& defaultValue)
{
	if (paramaters.find(paramaterName) != paramaters.end())
	{
		return vec4FromColor( paramaters[paramaterName].ColorFactor() );
	}
	return  defaultValue;
}

std::string Redline::GLTFSceneLoader::TryGetMaterialString(tinygltf::ParameterMap & paramaters, const char * paramaterName, const std::string & defaultValue)
{
	if (paramaters.find(paramaterName) != paramaters.end())
	{
		return paramaters[paramaterName].string_value;
	}
	return  defaultValue;
}

bool Redline::GLTFSceneLoader::TryGetMaterialBool(tinygltf::ParameterMap & paramaters, const char * paramaterName, bool defaultValue)
{
	if (paramaters.find(paramaterName) != paramaters.end())
	{
		return paramaters[paramaterName].bool_value;
	}
	return  defaultValue;
}

int GLTFSceneLoader::TryGetTextureID(tinygltf::ParameterMap & paramaters, const char* paramaterName)
{
	if (paramaters.find(paramaterName) != paramaters.end())
	{
		return paramaters[paramaterName].TextureIndex();
	}
	return -1;
}

shared_ptr<Bitmap2D> GLTFSceneLoader::TryGetTexture(
	tinygltf::ParameterMap& paramaters,
	const std::vector<tinygltf::Texture>& textures,
	const vector<shared_ptr<Bitmap2D>>& images,
	const char* paramaterName)
{
	const int textureID = TryGetTextureID(paramaters, paramaterName);
	if (textureID != -1 && textureID < images.size())
	{
		auto sourceImage = textures[textureID].source;
		return images[sourceImage];
	}
	return nullptr;
}

