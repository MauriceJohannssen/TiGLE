#version 330 core

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec4 brightColor;
layout(location = 2) out vec4 vertexPosition;

//Object
in vec2 vUVs;
in vec3 vNormal;
in vec3 vFragPosition;
in vec4 vFragLightPosition;

uniform sampler2D shadowMap;

//Material
struct Material {
	sampler2D texture_diffuse1;
	sampler2D texture_diffuse2;
	sampler2D texture_diffuse3;
	sampler2D texture_specular1;
	sampler2D texture_specular2;
};

uniform Material material;

//Lighting
struct PointLight {
	vec3 position;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float constant;
	float linear;
	float quadratic;

	float intensity;
};

#define POINT_LIGHT_COUNT 3
uniform PointLight pointLights[POINT_LIGHT_COUNT];

struct DirectionalLight{
	vec3 direction;

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float intensity;
};

uniform DirectionalLight directionalLight;

//Camera
uniform vec3 cameraPosition;
uniform float bloomThreshold;

vec3 CalculateDirectionalLight(DirectionalLight dirLight, vec3 normal, vec3 viewDirection);
vec3 CalculatePointLight(PointLight pointLight, vec3 pNormal, vec3 pFragPosition, vec3 pViewDirection);
float CalculateShadow(vec4 fragPositionLightSpace, vec3 normal, vec3 lightDirection);

void main() {
	vec3 finalColor = vec3(0,0,0);

	finalColor += CalculateDirectionalLight(directionalLight, normalize(vNormal), cameraPosition - vFragPosition);

	for(int i = 0; i < POINT_LIGHT_COUNT; i++) {
		//finalColor += CalculatePointLight(pointLights[i], normalize(vNormal), vFragPosition, cameraPosition - vFragPosition);
	}
	fragColor = vec4(finalColor,1);

	//Check on brightness
	float brightness = dot(finalColor.rgb, vec3(0.2126, 0.7152, 0.0722));

	//This value determines what will be "considered" bloom.
	if(brightness > bloomThreshold) {
		brightColor = vec4(finalColor,1);
	}
	else{
		brightColor = vec4(0,0,0,1);
	}

	vertexPosition = vec4(vFragPosition,1);
}

vec3 CalculateDirectionalLight(DirectionalLight dirLight, vec3 normal, vec3 viewDirection) {
	vec3 ambient = vec3(texture(material.texture_diffuse1, vUVs)) * dirLight.ambient * 0.1;
	vec3 lightDirection = normalize(-dirLight.direction);

	vec3 diffuse = max(dot(normal, lightDirection), 0.0) * vec3(texture(material.texture_diffuse1, vUVs)) * dirLight.diffuse;

	vec3 halfwayDirection = normalize(lightDirection + viewDirection);
	float finalSpecular = pow(max(dot(normal, halfwayDirection), 0.0), 32);
	vec3 specular = finalSpecular * vec3(texture(material.texture_specular1, vUVs)) * dirLight.specular;

	float shadowValue = CalculateShadow(vFragLightPosition, normal, lightDirection);

	return (ambient + ((diffuse + specular) * (1 - shadowValue))) * dirLight.intensity;
}


vec3 CalculatePointLight(PointLight pointLight, vec3 pNormal, vec3 pFragPosition, vec3 pViewDirection) {
	//Ambient
	pViewDirection = normalize(pViewDirection);
	//Todo: Currently the ambient value is lowered here. This should be a variable!
	vec3 ambient = vec3(texture(material.texture_diffuse1, vUVs)) * pointLight.ambient * 0.2;
	
	//Diffuse
	vec3 lightDirection = normalize(pointLight.position - vFragPosition);
	vec3 diffuse = max(dot(pNormal, lightDirection), 0.0) * vec3(texture(material.texture_diffuse1, vUVs)) * pointLight.diffuse;
	
	//Specular
	//vec3 reflectedLightDirection = reflect(-lightDirection, pNormal);
	vec3 halfwayDirection = normalize(lightDirection + pViewDirection);
	float finalSpecular = pow(max(dot(pNormal, halfwayDirection),0.0), 32);
	vec3 specular = (finalSpecular * vec3(texture(material.texture_specular1, vUVs))) * pointLight.specular;
	
	//Attenuation
	float distanceLightFrag = length(pointLight.position - vFragPosition);
	float attenuation = 1.0 / (pointLight.constant + pointLight.linear * distanceLightFrag + pointLight.quadratic * (distanceLightFrag * distanceLightFrag));

	float shadowValue = 0; //CalculateShadow(vFragLightPosition, pNormal, lightDirection);
	return (ambient + ((1 - shadowValue) * (diffuse + specular))) * attenuation * pointLight.intensity;
}

float CalculateShadow(vec4 fragPositionLightSpace, vec3 normal, vec3 lightDirection){
	vec3 coords = fragPositionLightSpace.xyz / fragPositionLightSpace.w;
	coords = coords * 0.5 + 0.5;

	float closestDepth = texture(shadowMap, coords.xy).r;
	float currentDepth = coords.z;
	if(currentDepth > 1.0){
		return 0.0;
	}

	float bias = max(0.05 * (1.0 - dot(normal, lightDirection)), 0.006);

	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

	//PCF
	for(int x = -2; x <= 2; x++){
		for(int y = -2; y <= 2; y++){
			float pcfDepth = texture(shadowMap, coords.xy + vec2(x,y) * texelSize).r;
			shadow += currentDepth - bias > pcfDepth ? 1.0 : 0.0;
		}
	}

	shadow /= 25; //Get the weighted average.
	return shadow;
}
