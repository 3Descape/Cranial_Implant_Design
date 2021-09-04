#ifndef TRIANGLE_H
#define TRIANGLE_H

typedef struct BarycentricTriangle
{
    glm::vec3 p0;
    glm::vec3 edge0;
    glm::vec3 edge1;
    glm::vec3 normal;

    BarycentricTriangle(const glm::vec3& point0, const glm::vec3& point1, const glm::vec3& point2) :
        p0(point0), edge0(point2 - point0), edge1(point1 - point0), normal(glm::normalize(glm::cross(edge0, edge1))) {}
    BarycentricTriangle(const BarycentricTriangle& other) = default;
    BarycentricTriangle& operator=(const BarycentricTriangle&) = default;
    ~BarycentricTriangle() {}
} BarycentricTriangle;

#endif // TRIANGLE_H