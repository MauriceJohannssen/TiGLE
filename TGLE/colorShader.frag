#version 330 core

out vec4 fragColor;

//Object
in vec3 vNormal;
in vec3 vFragPosition;
in vec2 vUVs;

//Material
struct Material
{
	sampler2D texture_diffuse1;
	sampler2D texture_diffuse2;
	sampler2D texture_diffuse3;
	sampler2D texture_specular1;
	sampler2D texture_specular2;
};

uniform Material material;

//Lighting
struct Light
{
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;
};

uniform Light light;

//Camera
uniform vec3 cameraPosition;

void main()
{
	vec3 ambient = vec3(texture(material.texture_diffuse1, vUVs)) * light.ambient;

	vec3 normal = normalize(vNormal);
	vec3 lightDirection = normalize(light.position - vFragPosition);
	vec3 diffuse = (max(dot(normal, lightDirection), 0.0) * vec3(texture(material.texture_diffuse1, vUVs))) * light.diffuse;

	vec3 viewDirection = normalize(cameraPosition - vFragPosition);
	vec3 reflectedLightDirection = reflect(-lightDirection, normalize(vNormal));
	float finalSpecular = pow(max(dot(viewDirection, reflectedLightDirection),0.0), 64);
	vec3 specular = (finalSpecular * vec3(texture(material.texture_specular1, vUVs).r)) * light.specular;

	//Point light
	//Attenuation
	float distanceLightFrag = length(light.position - vFragPosition);
	float attenuation = 1.0 / (light.constant + light.linear * distanceLightFrag + light.quadratic * (distanceLightFrag * distanceLightFrag));

	vec3 finalColor = (ambient + diffuse + specular) * attenuation;

	fragColor = vec4(finalColor,1);
}