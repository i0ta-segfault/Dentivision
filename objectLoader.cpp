#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include <unordered_map>

void loadObj(const char* filepath, std::vector<float>& vertices, std::vector<unsigned int>& indices, std::vector<float>& colors, std::vector<float>& texCoords) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filepath))
        throw std::runtime_error(warn + err);

    // Map vertex indices to their texture coordinates and colors
    std::unordered_map<size_t, size_t> vertexIndexMap;
    size_t currentIndex = 0;

    for (const auto& shape : shapes) {
        float r = .95f, g = .95f, b = .9f;   // default color to whitish
        if (shape.name == "wet") {
            r = 1.0f; g = 0.75f; b = 0.8f;
        } else if (shape.name == "mouth") {
            r = 0.55f; g = 0.2f; b = 0.2f;
        }

        for (const auto& index : shape.mesh.indices) {
            size_t vertexIndex = index.vertex_index;
            size_t texCoordIndex = index.texcoord_index;

            if (vertexIndexMap.find(vertexIndex) == vertexIndexMap.end()) {
                // Store vertex coordinates
                vertices.push_back(attrib.vertices[3 * vertexIndex + 0]);
                vertices.push_back(attrib.vertices[3 * vertexIndex + 1]);
                vertices.push_back(attrib.vertices[3 * vertexIndex + 2]);

                // Store texture coordinates if available
                if (texCoordIndex >= 0) {
                    texCoords.push_back(attrib.texcoords[2 * texCoordIndex + 0]);
                    texCoords.push_back(attrib.texcoords[2 * texCoordIndex + 1]);
                } 
                else {
                    texCoords.push_back(0.0f); // Default texture coordinates if none provided
                    texCoords.push_back(0.0f);
                }

                // Store color
                colors.push_back(r);
                colors.push_back(g);
                colors.push_back(b);

                // Map vertex index to its new index in the buffers
                vertexIndexMap[vertexIndex] = currentIndex;
                ++currentIndex;
            }

            // Add index to indices buffer (use the mapped index)
            indices.push_back(vertexIndexMap[vertexIndex]);
        }
    }
}
