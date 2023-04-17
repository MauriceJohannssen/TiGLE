#version 330 core
out vec4 fragColor;

in vec2 vUVs;

uniform sampler2D screenTexture;

void main()
{
   //float depthValue = texture(screenTexture, vUVs).r;
   //fragColor = vec4(vec3(depthValue,0,0), 1);
   fragColor = texture(screenTexture, vUVs);
} 