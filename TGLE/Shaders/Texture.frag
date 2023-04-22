#version 460 core
out vec4 fragColor;

in vec2 vUVs;

uniform sampler2D screenTexture;
uniform sampler2DArray CSM;

void main()
{
	 //float depthValue = texture(CSM, vec3(vUVs, 2)).r;
	vec4 color = texture(screenTexture, vUVs);
	fragColor = color;
} 