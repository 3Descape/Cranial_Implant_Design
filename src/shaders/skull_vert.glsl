#version 420 core
layout (location = 0) in vec3 i_position;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

out Vertex
{
    vec3 v_fragment_position;
    flat float v_is_masked_selection;
    flat float v_is_masked_region;
    vec3 i_position;
} vertex;

void main()
{
    mat4 mv = u_view * u_model;
    gl_Position = u_projection * mv * vec4(i_position, 1.0);
    vertex.v_fragment_position = vec3(u_model * vec4(i_position, 1.0));
    vertex.i_position = i_position;
    vertex.v_is_masked_selection = 0.0;
    vertex.v_is_masked_region = 0.0;
}