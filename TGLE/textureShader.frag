#version 330 core

in vec2 vTexCoords;
out vec4 fragColor;

uniform sampler2D inTexture;
uniform vec3 lightColor;

void main()
{
	float ambientStrength = 0.1;
	vec3 ambient = ambientStrength * lightColor;
	fragColor = texture(inTexture, vTexCoords) * vec4(ambient,1);
}