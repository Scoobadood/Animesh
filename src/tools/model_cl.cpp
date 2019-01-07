#include "model_cl.hpp"
#include <iostream>


void Model::data(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices ) const {
    vertices.clear();
    indices.clear();
    unsigned int baseIdx = 0;
    for(unsigned int i = 0; i < meshes.size(); i++) {
        for( auto v : meshes[i].vertices) {
            vertices.push_back(v);
        }
        for( auto idx : meshes[i].indices) {
            indices.push_back(idx + baseIdx);
        }
        baseIdx = vertices.size();
    }
}


void Model::loadModel(const std::string& path) {
	using namespace std;

	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
 	if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        cout << "ERROR::ASSIMP::" << importer.GetErrorString() << endl;
        return;
    }
    this->directory = path.substr(0, path.find_last_of('/'));

    processNode(scene->mRootNode, scene);
}  

void Model::processNode(aiNode *node, const aiScene *scene) {
    std::cout << "Model::processNode" << std::endl;

    // process all the node's meshes (if any)
    for(unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]]; 
        this->meshes.push_back(processMesh(mesh, scene));			
    }

    // then do the same for each of its children
    for(unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene) {
    using namespace cl;
    using namespace std;

    cout << "Model::processMesh" << endl;

	vector<Vertex> vertices;
    vector<unsigned int> indices;

    for(unsigned int i = 0; i < mesh->mNumVertices; i++) {
        cl_float3 vector; 
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z; 

        Vertex vertex;
		vertex.position = vector;

		vector.x = mesh->mNormals[i].x;
		vector.y = mesh->mNormals[i].y;
		vector.z = mesh->mNormals[i].z;
		vertex.normal = vector; 

        vertices.push_back(vertex);
    }

    // process indices
	for(unsigned int i = 0; i < mesh->mNumFaces; i++) {
	    aiFace face = mesh->mFaces[i];
	    for(unsigned int j = 0; j < face.mNumIndices; j++)
	        indices.push_back(face.mIndices[j]);
	}  

    return Mesh(vertices, indices);	
}