#include <limits>
#include <glm/glm.hpp>

#include "Triangle.hpp"

#ifndef RAY_H
#define RAY_H

typedef struct IntersectionPoint
{
    glm::vec3 point;
    glm::vec3 normal;
    float t;
    bool is_valid;

    IntersectionPoint(bool valid) : is_valid(valid), t(std::numeric_limits<float>::max()) {}
    IntersectionPoint& operator=(const IntersectionPoint&) = default;
} IntersectionPoint;

typedef struct Ray
{
    const glm::vec3 origin;
    const glm::vec3 direction;

    Ray(const glm::vec3& origin, const glm::vec3& direction) : origin(origin), direction(direction) {}
} Ray;

inline IntersectionPoint rayTriangleIntersect(const Ray& ray, const BarycentricTriangle& triangle)
{
    glm::vec3 pvec = glm::cross(ray.direction, triangle.edge1);
    float det = glm::dot(triangle.edge0, pvec);
    // if the determinant is negative the triangle is backfacing
    // if the determinant is close to 0, the ray misses the triangle
    const float kEpsilon = 1e-7;
    if (det < kEpsilon) return IntersectionPoint(false);

    float invDet = 1 / det;

    glm::vec3 tvec = ray.origin - triangle.p0;
    float u = glm::dot(tvec, pvec) * invDet;
    if (u < 0 || u > 1) return IntersectionPoint(false);

    glm::vec3 qvec = glm::cross(tvec, triangle.edge0);
    float v = glm::dot(ray.direction, qvec) * invDet;
    if (v < 0 || u + v > 1) return IntersectionPoint(false);

    IntersectionPoint intersection(true);
    intersection.t = glm::dot(triangle.edge1, qvec) * invDet;
    intersection.point = ray.origin + ray.direction * intersection.t;
    intersection.normal = triangle.normal;

    return intersection;
}

#endif // RAY_H