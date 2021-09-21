#version 330 core

in vec2 vTexCoords;
out vec4 fragColor;

uniform sampler2D inTexture;
uniform vec3 lightColor;

void main()
{
	fragColor = texture(inTexture, vTexCoords) * vec4(lightColor,1);
}