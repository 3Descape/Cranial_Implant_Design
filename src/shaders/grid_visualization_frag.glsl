#version 420 core

flat in uint v_level;

out vec4 FragColor;

const vec4 node_colors[4] = {
    vec4(0.00608299, 0.279541, 0.625, 1.0), // leaf nodes
    vec4(0.871, 0.394, 0.01916, 1.0),       // intermediate internal node levels
    vec4(0.0432, 0.33, 0.0411023, 1.0),     // first internal node level
    vec4(0.33, 0.38, 0.39, 1.0)             // root
};

void main()
{
    FragColor = node_colors[v_level];
}