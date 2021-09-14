#version 330 core

in vec3 vColor;
in vec2 vTexCoords;
out vec4 fragColor;

uniform sampler2D inTexture;

void main()
{
	fragColor = texture(inTexture, vTexCoords) * vec4(vColor, 1.0f);
}