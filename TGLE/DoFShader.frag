#version 330 core

out vec4 fragColor;
uniform sampler2D sharpTexture;
uniform sampler2D blurredTexture;
uniform sampler2D vertexPositions;
in vec2 vUVs;

vec2 nearFar = vec2(0.1, 1.0);

void main()
{
	float minDistance =  0.1;
	float maxDistance = 1.0;

	vec4 focusColor = texture(sharpTexture, vUVs);

	fragColor = focusColor;

	vec4 position = texture(vertexPositions, vUVs);

	if (position.a <= 0) { fragColor = vec4(1.0); return; }

	vec4 outOfFocusColor = texture(blurredTexture, vUVs);
	vec4 focusPoint = texture(vertexPositions, vec2(0.5, 0.5));
	float blur = smoothstep(minDistance, maxDistance, length(position - focusPoint));

	fragColor = mix(focusColor, outOfFocusColor, blur);
}