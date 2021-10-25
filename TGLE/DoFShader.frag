#version 330 core

out vec4 fragColor;

uniform sampler2D sharpTexture;
uniform sampler2D blurredTexture;
uniform sampler2D vertexPositions;

in vec2 vUVs;

void main()
{
	vec2 nearFar = vec2(0.5, 2.0);
	
	//Sample both sharp and blurred textures
	vec4 focusColor = texture(sharpTexture, vUVs);
	vec4 outOfFocusColor = texture(blurredTexture, vUVs);
	
	vec4 position = texture(vertexPositions, vUVs);
	//if (position.a <= 0) { fragColor = vec4(1.0); return; }
	vec4 focusPoint = texture(vertexPositions, vec2(0.5, 0.5));
	
	float blur = smoothstep(nearFar.x, nearFar.y, length(position - focusPoint));

	fragColor = mix(focusColor, outOfFocusColor, blur);
	
	//Shows amount of blur
	//fragColor = vec4(blur);
}