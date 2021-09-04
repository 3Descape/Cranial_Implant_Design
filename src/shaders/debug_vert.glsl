#version 420 core

out vec4 Color;

void main()
{
    vec4 positions[4];
    positions[0] = vec4(-1, 1, 0, 1);
    positions[1] = vec4(-1, -1, 0, 1);
    positions[2] = vec4(1, -1, 0, 1);
    positions[3] = vec4(1, 1, 0, 1);

    vec2 uv[4];
    uv[0] = vec2(0, 0);
    uv[1] = vec2(0, 1);
    uv[2] = vec2(1, 1);
    uv[3] = vec2(1, 0);

    gl_Position = positions[gl_VertexID];
    Color = vec4(uv[gl_VertexID], 0, 0);
}