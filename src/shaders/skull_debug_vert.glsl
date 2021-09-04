#version 420 core
layout (location = 0) in vec3 i_position;

out Vertex
{
    vec3 v_fragment_position;
    flat float v_is_masked_selection;
    flat float v_is_masked_region;
    vec3 i_position;
} vertex;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

uniform float u_area_picker_radius;
uniform vec3 u_area_picker_position;

uniform int u_clipping_mode; // 0 = LLS, 1 = LLS_CLIPPED_SELECTION, 2 = LLS_CLIPPING_AREAPICKER
uniform float u_user_selection_radius;
uniform bool u_user_selection_show;
uniform bool u_region_selection_show;
uniform uint u_user_selection_points_count;
uniform vec3 u_user_selection_points[512];

float distSquared( vec3 a, vec3 b) {
    vec3 c = a - b;
    return dot(c, c);
}

void main()
{
    mat4 mv = u_view * u_model;
    gl_Position = u_projection * mv * vec4(i_position, 1.0);
    vertex.v_fragment_position = vec3(u_model * vec4(i_position, 1.0));
    vertex.i_position = i_position;

    vertex.v_is_masked_region = float(u_region_selection_show && distSquared(vertex.v_fragment_position, u_area_picker_position) <= u_area_picker_radius * u_area_picker_radius);

    vertex.v_is_masked_selection = 0.0;
    for(int i = 0; i < u_user_selection_points_count && !bool(vertex.v_is_masked_selection) && (u_clipping_mode == 1 || u_user_selection_show); ++i) {
        vertex.v_is_masked_selection = float(distSquared(vertex.v_fragment_position, u_user_selection_points[i]) <= (u_user_selection_radius * u_user_selection_radius));
    }
}