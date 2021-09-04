#version 420 core
layout (location = 0) in vec3 i_position;
layout (location = 3) in uint i_level;

flat out uint v_level;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(i_position, 1.0);
    v_level = i_level;
}