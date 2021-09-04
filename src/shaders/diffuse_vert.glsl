#version 420 core
layout (location = 0) in vec3 aPos;
layout (location = 2) in vec3 aNormal;

out vec3 Normal;
out vec3 FragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    mat4 mv = view * model;
    gl_Position = projection * mv * vec4(aPos, 1.0);
    mat3 norm_matrix = transpose(inverse(mat3(mv)));
    Normal = normalize(norm_matrix * aNormal);
    FragPos = vec3(model * vec4(aPos, 1.0));
}