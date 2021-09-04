#include <vector>
#include <stdint.h>

#ifndef UTIL_PRIMITIVES_H
#define UTIL_PRIMITIVES_H

inline void generateSphere(int u_count, int v_count, std::vector<float>& data, std::vector<glm::uvec3>& indices)
{
    data.reserve((u_count + 1) * (v_count + 1) * 3 * 2);
    indices.reserve(u_count * v_count * 2);

    #define data_add_position_normal(data, x, y, z, nx, ny, nz) data.push_back(x); \
    data.push_back(y); \
    data.push_back(z); \
    data.push_back(nx); \
    data.push_back(ny); \
    data.push_back(nz)

    const float u_step = 2 * M_PI / u_count;
    const float v_step = M_PI / v_count;

    for(int v = 0; v <= v_count; ++v)
    {
        const float v_angle = M_PI / 2 - v * v_step; // starting from pi/2 to -pi/2
        const float xy = cosf(v_angle); // r * cos(u)
        const float z = sinf(v_angle); // r * sin(u)

        // add (sectorCount+1) vertices per stack
        for(int u = 0; u <= u_count; ++u)
        {
            const float u_angle = u * u_step; // starting from 0 to 2pi

            // vertex position (x, y, z)
            const float x = xy * cosf(u_angle); // r * cos(u) * cos(v)
            const float y = xy * sinf(u_angle); // r * cos(u) * sin(v)

            // for unit sphere unit normal direction == vertex position
            data_add_position_normal(data, x, y, z, x, y, z);
        }
    }

    // p0 -- p3
    // |  \  |
    // p1 -- p2
    for(int v = 0; v < v_count; ++v)
    {
        for(int u = 0; u < u_count; ++u)
        {
            uint32_t p0 = v * (u_count + 1) + u;
            uint32_t p1 = (v + 1) * (u_count + 1) +  u;
            uint32_t p2 = p1 + 1;
            uint32_t p3 = p0 + 1;

            indices.push_back(glm::uvec3(p0, p1, p2));
            indices.push_back(glm::uvec3(p0, p2, p3));
        }
    }

    #undef data_add_position_normal
}

#endif // UTIL_PRIMITIVES_H