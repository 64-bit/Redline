#pragma once
#include <tiny_gltf.h>

namespace tinygltf
{
	class Model;
}

namespace Redline
{
	class Scene;
	class Bitmap2D;
	class Material;
	class TriangleMesh;
	class SceneObject;

	class GLTFSceneLoader
	{
	public:
		std::shared_ptr<Scene> LoadSceneFromGLTFFile(const std::string& filename);
	private:

		std::vector<std::shared_ptr<Bitmap2D>> Images;
		std::vector<std::shared_ptr<Material>> Materials;
		std::vector<std::shared_ptr<TriangleMesh>> Meshes;

		static void LoadImages(tinygltf::Model& gltfFile, std::vector<std::shared_ptr<Bitmap2D>>& imagesOut);

		static void LoadMaterials(tinygltf::Model& gltfFile,
			const std::vector<std::shared_ptr<Bitmap2D>>& images,
			std::vector<std::shared_ptr<Material>>& materialsOut);

		static void LoadMeshes(tinygltf::Model& gltfFile,
			std::vector<std::shared_ptr<TriangleMesh>>& meshesOut);

		void FillScene(tinygltf::Model& gltfFile, std::shared_ptr<Scene>& scene);

		void CreateNode(tinygltf::Model& gltfFile, tinygltf::Node& gltfNode, std::shared_ptr<SceneObject> parent, std::shared_ptr<Scene>& scene);

		static float TryGetMaterialFloat(tinygltf::ParameterMap& paramaters, const char* paramaterName, float defaultValue);
		static mathfu::vec4 TryGetMaterialVector(tinygltf::ParameterMap& paramaters, const char* paramaterName, const mathfu::vec4& defaultValue);
		static std::string TryGetMaterialString(tinygltf::ParameterMap& paramaters, const char* paramaterName, const std::string& defaultValue);
		static bool TryGetMaterialBool(tinygltf::ParameterMap& paramaters, const char* paramaterName, bool defaultValue);

		static int TryGetTextureID(tinygltf::ParameterMap& paramaters, const char* paramaterName);
		static std::shared_ptr<Bitmap2D> TryGetTexture(tinygltf::ParameterMap& paramaters, const std::vector<tinygltf::Texture>& textures, const std::vector<std::shared_ptr<Bitmap2D>>& images, const char* paramaterName);


	};
}
