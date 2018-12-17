// Include GL Math
#include <glm/vec3.hpp>
#include <glm/vec2.hpp>

#include <vector>
#include <string>
#include "shader.hpp"

// Mesh classes
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};


struct Texture {
    GLuint id;
    std::string type;
};

class Mesh {
    public:
        /*  Mesh Data  */
        std::vector<Vertex> vertices;
        std::vector<unsigned int> indices;
        std::vector<Texture> textures;
        /*  Functions  */
        Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);
        void draw(Shader shader);
    private:
        /*  Render data  */
        GLuint VAO, VBO, EBO;
        /*  Functions    */
        void setupMesh();
};

class Model {
    public:
        /*  Functions   */
        Model(char *path) {
            loadModel(path);
        }
        void draw(Shader shader);   
    private:
        /*  Model Data  */
        vector<Mesh> meshes;
        string directory;
        /*  Functions   */
        void loadModel(string path);
        void processNode(aiNode *node, const aiScene *scene);
        Mesh processMesh(aiMesh *mesh, const aiScene *scene);
        vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName);
};