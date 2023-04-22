#version 460 core
layout(location = 0) in vec3 inPosition;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main()
{
	gl_Position = lightSpaceMatrix * model * vec4(inPosition, 1.0);
}