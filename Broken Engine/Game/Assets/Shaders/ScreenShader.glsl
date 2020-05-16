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
uniform sampler2D bloomBlurTexture;

uniform bool u_UseBloom = true;

uniform float u_GammaCorrection = 1.8;
uniform float u_HDR_Exposure = 1.0;
uniform bool u_UseHDR = true;

out vec4 FragColor;
in vec4 gl_FragCoord;

void main()
{
    vec3 textureOutput = texture(screenTexture, gl_FragCoord.xy / textureSize(screenTexture, 0)).rgb;
	vec3 bloomTextureOutput = texture(bloomBlurTexture, gl_FragCoord.xy / textureSize(bloomBlurTexture, 0)).rgb;
	vec3 finalColor = vec3(1.0);
	
	if(u_UseBloom)
		textureOutput += bloomTextureOutput;	

	// --- HDR Application (or not) ---
	if(u_UseHDR == true)
	{
		//reinhard tone mapping with gamma correction
		//vec3 mapped = texOutput/(texOutput+vec3(1.0));
		vec3 mapped = vec3(1.0) - exp(-textureOutput * u_HDR_Exposure);
		finalColor = pow(mapped, vec3(1.0/u_GammaCorrection));		
	}
	else
		finalColor = pow(textureOutput, vec3(1.0/u_GammaCorrection));


	// --- Output Fragment Color ---
	FragColor = vec4(finalColor, 1.0);
}
#endif //FRAGMENT_SHADER