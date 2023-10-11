
#include <intrin.h>
#include <glad/gl.h>
#include <glm/glm.hpp>

#ifndef UTIL_OPENGL
#define UTIL_OPENGL

#ifdef DEBUG_BUILD
#define OPENGL_CHECK(X) X; {GLenum err; \
while((err = glGetError()) != GL_NO_ERROR) { \
    std::cout << "GLERROR: " << __FILE__ << ", " << __LINE__ << ", Error: " << std::hex << err << std::dec << std::endl; __debugbreak(); }}
#else
#define OPENGL_CHECK(X) X
#endif // DEBUG_BUILD

inline glm::mat4 xyzToxzy(const glm::mat4& matrix)
{
    glm::mat4 xyz_to_xzy = glm::mat4();
    xyz_to_xzy[0][0] = 1.0f;
    xyz_to_xzy[1][2] = -1.0f;
    xyz_to_xzy[2][1] = 1.0f;
    xyz_to_xzy[3][3] = 1.0f;

    return xyz_to_xzy * matrix;
}

inline glm::vec3 xyzToxzy(const glm::vec3& vec)
{
    return glm::vec3(vec.x, vec.z, -vec.y);
}

inline glm::vec3 xzyToxyz(const glm::vec3& vec)
{
    return glm::vec3(vec.x, -vec.z, vec.y);
}

inline glm::vec3 viewToWorldSpace(float x_normalized, float y_normalized, const glm::mat4& view_project)
{
    // HOMOGENEOUS SPACE
    glm::vec4 screen_position = glm::vec4(x_normalized, -y_normalized, -1, 1);
    // Projection/Eye Space
    glm::mat4 view_project_inverse = inverse(view_project);
    glm::vec4 world_position = view_project_inverse * screen_position;

    world_position /= world_position.w;

    return xzyToxyz(glm::vec3(world_position));
}

#endif // UTIL_OPENGL