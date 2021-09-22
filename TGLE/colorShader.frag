#version 330 core

out vec4 fragColor;

//Object
in vec3 vNormal;
in vec3 vFragPosition;
in vec2 vUVs;

//Material
struct Material
{
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};

uniform Material material;

//Lighting
struct Light
{
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};

uniform Light light;

//Camera
uniform vec3 cameraPosition;

void main()
{

	vec3 ambient = vec3(texture(material.diffuse, vUVs)) * light.ambient;

	vec3 normal = normalize(vNormal);
	vec3 lightDirection = normalize(light.position - vFragPosition);
	vec3 diffuse = (max(dot(normal, lightDirection), 0.0) * vec3(texture(material.diffuse, vUVs))) * light.diffuse;

	vec3 viewDirection = normalize(cameraPosition - vFragPosition);
	vec3 reflectedLightDirection = reflect(-lightDirection, normalize(vNormal));
	float finalSpecular = pow(max(dot(viewDirection, reflectedLightDirection),0.0), material.shininess);
	vec3 specular = (finalSpecular * vec3(texture(material.specular, vUVs).r)) * light.specular;

	vec3 finalColor = ambient + diffuse + specular;

	fragColor = vec4(finalColor,1);
}