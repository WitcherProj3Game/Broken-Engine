#version 440 core
#define VERTEX_SHADER
#ifdef VERTEX_SHADER

layout (location = 0) in vec2 aPos;

void main()
{
	gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
}
#endif //VERTEX_SHADER

#define FRAGMENT_SHADER
#ifdef FRAGMENT_SHADER

uniform sampler2D screenTexture;
uniform sampler2D LUTTexture;

out vec4 FragColor;
in vec4 gl_FragCoord;

void main()
{
    vec3 textureColor = texture2D(screenTexture, gl_FragCoord.xy / textureSize(screenTexture, 0)).rgb;
	vec3 indexedColor = textureColor * 15;
    // Coordinates inside the LUT texture
    ivec2 LUTPosition1;
	LUTPosition1.x = int((floor(indexedColor.b) * 16) + round(indexedColor.r));
	LUTPosition1.y = int(15 - round(indexedColor.g)); // y is inverted

	vec3 filteredColor1 = texelFetch(LUTTexture, LUTPosition1, 0).rgb;

	ivec2 LUTPosition2;
	LUTPosition2.x = int((ceil(indexedColor.b) * 16) + round(indexedColor.r));
	LUTPosition2.y = int(15 - round(indexedColor.g)); // y is inverted

	vec3 filteredColor2 = texelFetch(LUTTexture, LUTPosition2, 0).rgb;

	vec3 finalColor = mix(filteredColor1, filteredColor2, fract(indexedColor.b));
	// --- Output Fragment Color ---
	FragColor = vec4(finalColor, 1.0);
}
#endif //FRAGMENT_SHADER
