#version 330 core

out vec4 fragColor;
uniform vec3 objectColor;

//Lighting
uniform vec3 lightColor;
uniform vec3 lightPosition;

in vec3 vNormal;
in vec3 vFragPosition;

//Camera
uniform vec3 cameraPosition;

void main()
{
	float ambientStrength = 0.1;
	vec3 ambient = ambientStrength * lightColor;

	vec3 normal = normalize(vNormal);
	vec3 lightDirection = normalize(lightPosition - vFragPosition);
	vec3 diffuse = max(dot(normal, lightDirection), 0.0) * lightColor;

	float specularStrength = 0.5;
	vec3 viewDirection = normalize(cameraPosition - vFragPosition);
	vec3 reflectedViewDirection = reflect(-lightDirection, normalize(vNormal));
	float finalSpecular = pow(max(dot(viewDirection, reflectedViewDirection),0.0),64);
	vec3 specular = specularStrength * finalSpecular * lightColor;

	vec3 final = (ambient + diffuse + specular) * objectColor;
	fragColor = vec4(final,1);
}