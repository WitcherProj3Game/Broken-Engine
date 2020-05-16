#version 440 core
#define VERTEX_SHADER
#ifdef VERTEX_SHADER

layout (location = 0) in vec3 a_Position;
layout (location = 3) in vec2 a_TexCoord;

out vec2 v_TexCoords;

void main()
{
	v_TexCoords = a_TexCoord;
	gl_Position = vec4(a_Position, 1.0);
}
#endif //VERTEX_SHADER

#define FRAGMENT_SHADER
#ifdef FRAGMENT_SHADER

uniform sampler2D u_ImageToBlur;
uniform vec3 u_BlurWeights1 = vec3(0.2270270270, 0.1945945946, 0.1216216216);
uniform vec2 u_BlurWeights2 = vec2(0.0540540541, 0.0162162162);
uniform bool u_HorizontalPass = true;

in vec2 v_TexCoords;
out vec4 FragColor;
in vec4 gl_FragCoord;

void main()
{
	float BlurWeights[5] = float[](u_BlurWeights1.x, u_BlurWeights1.y, u_BlurWeights1.z, u_BlurWeights2.x, u_BlurWeights2.y);

	vec2 texture_offset = 1.0/textureSize(u_ImageToBlur, 0);
	vec3 finalColor = texture(u_ImageToBlur, v_TexCoords).rgb * BlurWeights[0];
	finalColor = texture(u_ImageToBlur, gl_FragCoord.xy / textureSize(u_ImageToBlur, 0)).rgb * BlurWeights[0];

	if(u_HorizontalPass)
	{
		for(int i = 1; i < 5 ; ++i) //For amount of blur passes
		{
			finalColor += texture(u_ImageToBlur, v_TexCoords + vec2(texture_offset.x*i, 0.0)).rgb * BlurWeights[i];
			finalColor += texture(u_ImageToBlur, v_TexCoords - vec2(texture_offset.x*i, 0.0)).rgb * BlurWeights[i];
		}
	}
	else
	{
		for(int i = 1; i < 5 ; ++i) //For amount of blur passes
		{
			finalColor += texture(u_ImageToBlur, v_TexCoords + vec2(0.0, texture_offset.y*i)).rgb * BlurWeights[i];
			finalColor += texture(u_ImageToBlur, v_TexCoords - vec2(0.0, texture_offset.y*i)).rgb * BlurWeights[i];
		}
	}

	// --- Output Fragment Color ---
	FragColor = vec4(finalColor, 1.0);
}
#endif //FRAGMENT_SHADER