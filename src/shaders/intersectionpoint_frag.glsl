#version 420 core

in vec4 v_color;
in vec3 v_normal;
in vec3 v_fragment_position;

out vec4 FragColor;

uniform bool u_highlighted;

const vec3 lightPos = vec3(0.0, 50.0, 30.0);
const vec3 lightColor = vec3(1.0, 1.0, 1.0);
const vec3 ambient = vec3(0.2, 0.2, 0.2);

void main()
{
    vec3 normal = normalize(v_normal);
    vec3 lightDir = normalize(lightPos - v_fragment_position);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    vec3 lambert = (ambient + diffuse);

    vec3 fragment_color = v_color.rgb;

    if(u_highlighted)
        fragment_color = mix(vec3(1.0, 1.0, 1.0), fragment_color, 0.4);

    FragColor = vec4(fragment_color * lambert, 1.0);
}