#pragma once

#include <string>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mesh_cl.hpp"

class Model {
    public:
        Model(char *path) {
            loadModel(path);
        }

        Model(std::string path) {
            loadModel(path);
        }
        
        void data(std::vector<Vertex>& vertices, std::vector<unsigned int>& indices ) const;

    private:
        std::vector<Mesh>    meshes;
        std::string          directory;

        void loadModel(const std::string& path);

        void processNode(aiNode *node, const aiScene *scene);

        Mesh processMesh(aiMesh *mesh, const aiScene *scene);
};