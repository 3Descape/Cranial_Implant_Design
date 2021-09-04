#version 420 core

in vec3 Normal;
in vec3 FragPos;

out vec4 FragColor;

uniform vec4 color;

const vec3 lightPos = vec3(0.0, 50.0, 30.0);
const vec3 lightColor = vec3(1.0, 1.0, 1.0);
const vec3 ambient = vec3(.2, .2, .2);

void main()
{
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    vec3 result = (ambient + diffuse);

    FragColor = vec4(result, 1.0) * color;
}