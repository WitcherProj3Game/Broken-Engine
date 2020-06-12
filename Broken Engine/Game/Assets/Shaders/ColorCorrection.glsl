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
uniform samplerBuffer LUTData;

out vec4 FragColor;
in vec4 gl_FragCoord;

void main()
{
    vec3 normalizedTextureColor = texelFetch(screenTexture, ivec2(gl_FragCoord.xy), 0).rgb;
	vec3 indexedTextureColor = normalizedTextureColor * 15.0f;              //# of pixels per square -1 for 0 indexing

    // Coordinates inside the LUT texture
    int     redPosition     = int(round(indexedTexturecolor.r));
    int     greenPosition   = int(round(indexedTextureColor.g * 16 * 16));  //# of squares by # of pixels per square
    float   bluePosition    = indexedTextureColor.b * 16.0f;                //# of pixels per square, float for later mix

	int LUTPositionLow = int(floor(bluePosition)) + greenPosition + redPosition;
	vec3 filteredColorLow = texelFetch(LUTData, LUTPositionLow, 0).rgb;

	int LUTPositionHigh = int(ceil(bluePosition)) + greenPosition + redPosition;
	vec3 filteredColorHigh = texelFetch(LUTData, LUTPositionHigh, 0).rgb;

	vec3 finalColor = mix(filteredColorLow, filteredColorHigh, fract(bluePosition));

	// --- Output Fragment Color ---
	FragColor = vec4(finalColor, 1.0);
}
#endif //FRAGMENT_SHADER