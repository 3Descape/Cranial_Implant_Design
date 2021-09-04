#version 420 core
layout (location = 0) in vec3 aPos;

out vec4 v_color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

const int RAND_MAX = 32767;
uint next = uint(gl_VertexID);
float rand() {
    next =  next * 1103515245 + 12345;
    return float(uint(next/65536) % RAND_MAX) / float(RAND_MAX);
}

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    v_color = vec4(rand(), rand(), rand(), 1.0);
}