#include "SceneNode.hpp"

#include "cs488-framework/MathUtils.hpp"

#include <iostream>
#include <sstream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "mesh_obj.hpp"

using namespace glm;
using namespace std;
mesh_obj::mesh_obj(){

}

void mesh_obj::load_mesh(const std::string& pFile, const std::string& ptex){
  // Create an instance of the Importer class
  texture_path = ptex;

  Assimp::Importer importer;
  // And have it read the given file with some example postprocessing
  // Usually - if speed is not the most important aspect for you - you'll 
  // propably to request more postprocessing than we do in this example.
  const aiScene* scene = importer.ReadFile( pFile, 
        aiProcess_CalcTangentSpace       | 
        aiProcess_Triangulate            |
        aiProcess_JoinIdenticalVertices  |
        aiProcess_SortByPType);
  // If the import failed, report it
  if( scene){
  		aiMesh *current_mesh = scene->mMeshes[0];
  		for (int i = 0; i < current_mesh->mNumFaces; ++i){
  			aiFace current_face = current_mesh-> mFaces[i];
  			for (int j = 0; j < current_face.mNumIndices; ++j){
  				unsigned int a_v = current_face.mIndices[j];
  				vec2 uv_map(current_mesh->mTextureCoords[0][a_v].x,
  							      current_mesh->mTextureCoords[0][a_v].y);
  				UV_map.push_back(uv_map);

  				vec3 vpoints(double(current_mesh->mVertices[a_v].x),
  							       double(current_mesh->mVertices[a_v].y),
  							       double(current_mesh->mVertices[a_v].z));
  				Vpoints.push_back(vpoints);

  				vec3 vnormals(double(current_mesh->mNormals[a_v].x),
  							        double(current_mesh->mNormals[a_v].y),
  							        double(current_mesh->mNormals[a_v].z));
  				Vnormals.push_back(vnormals);
  			}

  		}
  }

}
