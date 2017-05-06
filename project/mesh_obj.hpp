#pragma once



#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>           // Output data structure
#include <assimp/postprocess.h>     // Post processing flags
#include <vector>


class mesh_obj{
public:
	std::vector<glm::vec2> UV_map;
	std::vector<glm::vec3> Vpoints;
	std::vector<glm::vec3> Vnormals;
	std::string texture_path;
	mesh_obj();
	void load_mesh(const std::string& pFile, const std::string& ptex);
};
