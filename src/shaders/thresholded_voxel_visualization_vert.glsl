#version 420 core
layout (location = 0) in vec3 i_position;
layout (location = 3) in float i_value;

out float v_value;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(i_position, 1.0);
    v_value = i_value;
}