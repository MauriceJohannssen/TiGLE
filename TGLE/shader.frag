#version 330 core

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec4 brightColor;
layout(location = 2) out vec4 vertexPosition;

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
struct PointLight
{
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;
};

#define POINT_LIGHT_COUNT 3  
uniform PointLight pointLights[POINT_LIGHT_COUNT];

//Camera
uniform vec3 cameraPosition;

vec3 CalcDirectionalLight(PointLight pointLight, vec3 pNormal, vec3 pFragPosition, vec3 pViewDirection);

void main()
{
	vec3 finalColor = vec3(0,0,0);
	for(int i = 0; i < POINT_LIGHT_COUNT; i++)
	{
		finalColor += CalcDirectionalLight(pointLights[i], normalize(vNormal), vFragPosition, cameraPosition - vFragPosition);
	}
	fragColor = vec4(finalColor,1);

	//Check on brightness
	float brightness = dot(finalColor.rgb, vec3(0.2126, 0.7152, 0.0722));

	//This value determines what will be "considered" bloom.
	if(brightness > 1.3)
		brightColor = vec4(finalColor,1);
	else
		brightColor = vec4(0,0,0,1);

	vertexPosition = vec4(vFragPosition,1);
}


vec3 CalcDirectionalLight(PointLight pointLight, vec3 pNormal, vec3 pFragPosition, vec3 pViewDirection)
{
	//Ambient
	pViewDirection = normalize(pViewDirection);
	vec3 ambient = vec3(texture(material.texture_diffuse1, vUVs)) * pointLight.ambient;
	//Diffuse
	vec3 lightDirection = normalize(pointLight.position - vFragPosition);
	vec3 diffuse = max(dot(pNormal, lightDirection), 0.0) * vec3(texture(material.texture_diffuse1, vUVs)) * pointLight.diffuse;
	//Specular
	vec3 reflectedLightDirection = reflect(-lightDirection, pNormal);
	float finalSpecular = pow(max(dot(pViewDirection, reflectedLightDirection),0.0), 32);
	vec3 specular = (finalSpecular * vec3(texture(material.texture_specular1, vUVs))) * pointLight.specular;
	//Attenuation
	float distanceLightFrag = length(pointLight.position - vFragPosition);
	float attenuation = 1.0 / (pointLight.constant + pointLight.linear * distanceLightFrag + pointLight.quadratic * (distanceLightFrag * distanceLightFrag));

	return (ambient + diffuse + specular) * attenuation;
}