#version 330 core

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoords;
layout(location = 3) in vec3 inNormal;

out vec3 vColor;
out vec2 vTexCoords;
out vec3 vNormal;
out vec3 vFragPosition;

uniform mat4 transform;
uniform mat4 objectMatrix;

void main()
{
	gl_Position = transform * vec4(inPosition, 1.0f);
	//vColor = inColor;
	//vTexCoords = inTexCoords;
	vNormal = inNormal;
	vFragPosition = vec3(objectMatrix * vec4(inPosition,1));
}