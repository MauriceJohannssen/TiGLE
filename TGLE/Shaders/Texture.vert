#version 330 core
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inUVs;

out vec2 vUVs;

void main()
{
    vUVs = inUVs;
    gl_Position = vec4(inPosition.x, inPosition.y, 0.0, 1.0); 
}  