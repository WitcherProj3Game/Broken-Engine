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

out vec4 FragColor;

uniform sampler2D screenTexture;
uniform vec2 screenTexture_size;
uniform float u_GammaCorrection = 1.8;
uniform float u_HDR_Exposure = 1.0;
uniform bool u_UseHDR = true;

void main()
{
	vec3 texOutput = texture(screenTexture, gl_FragCoord.xy / screenTexture_size).rgb;

	if(u_UseHDR == true)
	{
		//reinhard tone mapping with gamma correction
		//vec3 mapped = texOutput/(texOutput+vec3(1.0));

		vec3 mapped = vec3(1.0) - exp(-texOutput * u_HDR_Exposure);
		mapped = pow(mapped, vec3(1.0/u_GammaCorrection));

		FragColor = vec4(mapped, 1.0);
	}
	else
		FragColor = vec4(texOutput, 1.0);

}
#endif //FRAGMENT_SHADER
