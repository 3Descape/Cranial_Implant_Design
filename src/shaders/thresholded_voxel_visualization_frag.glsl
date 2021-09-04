#version 420 core

in float v_value;

out vec4 FragColor;

uniform float u_threshold;

void main()
{
    if(v_value < u_threshold)
        discard;

    FragColor = mix(vec4(0, 0, 1, 1), vec4(0, 1, 0, 1), v_value);
}