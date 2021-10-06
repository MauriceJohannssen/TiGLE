#version 330 core

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUVs;

out vec2 vUVs;
out vec3 vNormal;
out vec3 vFragPosition;

uniform mat4 transform;
uniform mat4 objectMatrix;

void main()
{
	gl_Position = transform * vec4(inPosition, 1.0);
	vUVs = inUVs;
	vNormal = inNormal;
	vFragPosition = vec3(objectMatrix * vec4(inPosition,1.0));
}