#pragma once
#include "Math/math.h"
namespace mortal
{
    struct Vertex
    {
        glm::vec3 Position;
        glm::vec3 Color;
        glm::vec2 TexCoord;
        glm::vec3 Normal;

        bool operator==(const Vertex& rhs) const {
            return Position == rhs.Position && Color == rhs.Color && Normal == rhs.Normal && TexCoord == rhs.TexCoord;
        }
    };
} // namespace mortal

template<>
struct std::hash<mortal::Vertex> {
    size_t operator()(mortal::Vertex const& vertex) const {
        return (hash<glm::vec3>{}(vertex.Position)) ^ 
            (hash<glm::vec3>{}(vertex.Color)) << 1  ^
            (hash<glm::vec3>{}(vertex.Normal)) << 2 ^
            (hash<glm::vec2>{}(vertex.TexCoord) << 3);
    }
};