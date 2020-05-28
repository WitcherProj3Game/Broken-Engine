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
    vec3 textureColor = texture(screenTexture, gl_FragCoord.xy / textureSize(screenTexture, 0)).rgb;
    

	// Calculate the indices inside LUT
	int redIndex = textureColor.r / 17;
    int greenIndex = textureColor.g / 17;
	int blueIndex = textureColor.b / 17;

    // Coordinates inside the LUT texture
    vec2 LUTPosition = (16 * blueIndex + redIndex, greenIndex);

	vec3 finalColor = texture(LUTTexture, LUTPosition).rgb;

	// --- Output Fragment Color ---
	FragColor = vec4(finalColor, 1.0);
}
#endif //FRAGMENT_SHADER