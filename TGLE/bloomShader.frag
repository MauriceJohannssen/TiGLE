#version 330 core

in vec2 vUVs;
layout(location = 1) out vec4 fragColor;

uniform sampler2D hdrRender;
uniform sampler2D bloomRender;

void main()
{
	const float gamma = 2.2;
    //This will be a uniform
    const float exposure = 0.3;
    vec3 colorHDR = texture(hdrRender, vUVs).rgb;
    vec3 bloomHDR = texture(bloomRender, vUVs).rgb;
    colorHDR += bloomHDR;
    vec3 mappedColorLDR = vec3(1.0) - exp(-colorHDR * exposure); 
    mappedColorLDR = pow(mappedColorLDR, vec3(1.0 / gamma));
    fragColor = vec4(mappedColorLDR, 1.0);
}
