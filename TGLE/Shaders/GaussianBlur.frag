#version 330 core

out vec4 fragColor;
in vec2 vUVs;

uniform sampler2D image;
uniform bool horizontal;
float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main() {
	vec2 texelOffset = 1.0 / textureSize(image, 0);
	vec3 result = texture(image, vUVs).rgb * weight[0];
	if(horizontal) {
		for(int i = 1; i < 5; i++) {
			result += texture(image, vUVs + vec2(texelOffset.x * i, 0.0)).rgb * weight[i];
			result += texture(image, vUVs - vec2(texelOffset.x * i, 0.0)).rgb * weight[i];
		}
	}
	else {
		for(int i = 1; i < 5; i++) {
			result += texture(image, vUVs + vec2(0.0, texelOffset.y * i)).rgb * weight[i];
			result += texture(image, vUVs - vec2(0.0, texelOffset.y * i)).rgb * weight[i];
		}
	}

	fragColor = vec4(result, 1.0);
}
