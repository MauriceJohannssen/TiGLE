#version 330 core

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec4 brightColor;

uniform vec3 lightColor;
uniform float intensity;
uniform float bloomThreshold;

void main() {
	vec3 light = lightColor * intensity;
	fragColor = vec4(light, 1);
	if(dot(light * 3, vec3(0.2126, 0.7152, 0.0722)) > bloomThreshold) {
		brightColor = vec4(light, 1);
	}
	else {
		brightColor = vec4(0,0,0,1);
	}
}