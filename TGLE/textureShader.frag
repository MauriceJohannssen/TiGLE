#version 330 core
out vec4 fragColor;

in vec2 vUVs;

uniform sampler2D screenTexture;

void main()
{
    const float gamma = 2.2;
    //This will be a uniform
    const float exposure = 0.5;
    vec3 colorHDR = texture(screenTexture, vUVs).rgb;
    vec3 mappedColorLDR = vec3(1.0) - exp(-colorHDR * exposure); 
    mappedColorLDR = pow(mappedColorLDR, vec3(1.0 / gamma));
    fragColor = vec4(mappedColorLDR, 1.0);
} 