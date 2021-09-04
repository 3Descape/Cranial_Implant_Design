#version 420 core
layout (location = 0) in vec3 i_position;
layout (location = 1) in vec4 i_color;
layout (location = 2) in vec3 i_normal;

out vec4 v_color;
out vec3 v_normal;
out vec3 v_fragment_position;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    mat4 mv = view * model;
    gl_Position = projection * mv * vec4(i_position, 1.0);
    mat3 normal_matrix = transpose(inverse(mat3(mv)));
    v_color = i_color;
    v_normal = normalize(normal_matrix * i_normal);
    v_fragment_position = vec3(model * vec4(i_position, 1.0));
}