#version 330 core

out vec4 fragColor;

//Object
in vec3 vNormal;
in vec3 vFragPosition;

//Material
struct Material
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};

uniform Material material;

//Lighting
uniform vec3 lightColor;
uniform vec3 lightPosition;

//Camera
uniform vec3 cameraPosition;

void main()
{
	vec3 ambient = material.ambient * lightColor;

	vec3 normal = normalize(vNormal);
	vec3 lightDirection = normalize(lightPosition - vFragPosition);
	vec3 diffuse = (max(dot(normal, lightDirection), 0.0) * material.diffuse) * lightColor;

	vec3 viewDirection = normalize(cameraPosition - vFragPosition);
	vec3 reflectedLightDirection = reflect(-lightDirection, normalize(vNormal));
	float finalSpecular = pow(max(dot(viewDirection, reflectedLightDirection),0.0), material.shininess);
	vec3 specular = (finalSpecular * material.specular) * lightColor;

	vec3 finalColor = ambient + diffuse + specular;
	fragColor = vec4(finalColor,1);
}