#pragma once

#include <string>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "mesh.hpp"

class Model {
    public:
        Model(char *path) {
            loadModel(path);
        }

        void draw(Shader shader);   

    private:
        std::vector<Mesh>    meshes;
        std::string          directory;
        std::vector<Texture> textures_loaded; 

        void loadModel(const std::string& path);

        void processNode(aiNode *node, const aiScene *scene);

        Mesh processMesh(aiMesh *mesh, const aiScene *scene);

        std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, const std::string& typeName);
};