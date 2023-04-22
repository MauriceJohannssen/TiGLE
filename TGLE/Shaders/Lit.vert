#version 460 core

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUVs;

out vec2 vUVs;
out vec3 vNormal;
out vec3 vFragPosition;
out vec4 vFragLightPosition;

uniform mat4 transform;
uniform mat4 objectMatrix;
uniform mat4 lightSpaceMatrix;

void main()
{
	vUVs = inUVs;
	vNormal = transpose(inverse(mat3(objectMatrix))) * inNormal;
	vFragPosition = vec3(objectMatrix * vec4(inPosition,1.0));
	vFragLightPosition = lightSpaceMatrix * vec4(vFragPosition, 1.0);
	gl_Position = transform * vec4(inPosition, 1.0);
}