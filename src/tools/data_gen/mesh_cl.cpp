#include "mesh_cl.hpp"

Mesh::Mesh( std::vector<Vertex> vertices, 
            std::vector<unsigned int> indices) {
	this->vertices = vertices;
    this->indices = indices;
}