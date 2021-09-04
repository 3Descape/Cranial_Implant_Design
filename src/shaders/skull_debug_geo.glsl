#version 420 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

uniform mat4 u_model;
uniform mat4 u_view;

out vec3 v_normal;
out vec3 v_fragment_position;
flat out float v_is_masked_selection;
flat out float v_is_masked_region;

in Vertex
{
    vec3 v_fragment_position;
    flat float v_is_masked_selection;
    flat float v_is_masked_region;
    vec3 i_position;
} vertex[];

void main()
{
    mat3 normal_matrix = transpose(inverse(mat3(u_view * u_model)));
    v_normal = normalize(normal_matrix * cross(vertex[2].i_position - vertex[0].i_position, vertex[1].i_position - vertex[0].i_position));

    for(int i = 0; i < gl_in.length(); ++i) {
        gl_Position = gl_in[i].gl_Position;
        v_is_masked_selection = vertex[i].v_is_masked_selection;
        v_is_masked_region = vertex[i].v_is_masked_region;
        v_fragment_position = vertex[i].v_fragment_position;
        EmitVertex();
    }

    EndPrimitive();
}