#version 330 core

out vec4 fragColor;

uniform sampler2D sharpTexture;
uniform sampler2D blurredTexture;
uniform sampler2D vertexPositions;

in vec2 vUVs;

uniform vec3 cameraPosition;
uniform float aperture, imageDistance, planeInFocus, near, far;

void main()
{
	float objectDistance = length(vec3(texture(vertexPositions, vUVs)) - cameraPosition);
	float zDistance = objectDistance; //-far * near / (objectDistance * (far - near) - far);

	float focalLength = planeInFocus + imageDistance;

	//Calculate circle of confusion
	float coc = abs(aperture * (focalLength * (zDistance - planeInFocus)) / (zDistance * (planeInFocus - focalLength)));
	
	//Sample both sharp and blurred textures
	vec4 focusColor = texture(sharpTexture, vUVs);
	vec4 outOfFocusColor = texture(blurredTexture, vUVs);

	float blur = smoothstep(near, far, coc);

	fragColor = mix(focusColor, outOfFocusColor, blur);
	
	//Shows amount of blur
	//fragColor = vec4(blur);
}