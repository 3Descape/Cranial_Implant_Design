#version 420 core

in vec3 v_normal;
in vec3 v_fragment_position;
flat in float v_is_masked_selection;
flat in float v_is_masked_region;

out vec4 FragColor;

uniform vec4 u_color;

const vec3 lightPos = vec3(0.0, 50.0, 30.0);
const vec3 lightColor = vec3(1.0, 1.0, 1.0);
const vec3 ambient = vec3(0.2, 0.2, 0.2);

void main()
{
    vec3 fragment_color = u_color.rgb;
    if(v_is_masked_region == 1.0)
        fragment_color = vec3(0.0, 1.0, 0.0);
    if(v_is_masked_selection == 1.0)
        fragment_color = vec3(1.0, 0.0, 0.0);
    vec3 normal = normalize(v_normal);
    vec3 lightDir = normalize(lightPos - v_fragment_position);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    vec3 lambert = (ambient + diffuse);

    FragColor = vec4(lambert * fragment_color, 1.0);
}