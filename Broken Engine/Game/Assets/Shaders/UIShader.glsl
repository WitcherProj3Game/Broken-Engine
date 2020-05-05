#version 440 core

#define VERTEX_SHADER
#ifdef VERTEX_SHADER

//Layout Daya
layout (location = 0) in vec3 a_Position;
layout (location = 3) in vec2 a_TexCoord;

//Uniforms
uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Proj;

uniform vec4 u_Color = vec4(1.0);

//Varyings
out vec2 v_TexCoord;
out vec4 v_Color;

void main()
{
	v_Color = u_Color;
	v_TexCoord = a_TexCoord;

	gl_Position = u_Proj * u_View * u_Model * vec4(a_Position, 1.0);
}

#endif //VERTEX_SHADER

#define FRAGMENT_SHADER
#ifdef FRAGMENT_SHADER

//Output Variables
out vec4 out_color;

//Input Variables (Varying)
in vec2 v_TexCoord;
in vec4 v_Color;

//Uniforms
uniform int u_UseTextures = 0;
uniform int u_HasTransparencies = 0;
uniform int u_IsText = 0;
uniform sampler2D u_AlbedoTexture;

uniform float u_GammaCorrection = 2.2;

//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
void main()
{
	if(u_IsText == 1)
	{
		vec4 sampled = vec4(1.0, 1.0, 1.0, texture(u_AlbedoTexture, v_TexCoord).r);
		out_color = v_Color * sampled;
		return;
	}

	// Transparency
	float alpha = 1.0;
	if(u_HasTransparencies == 1)
	{
		if(u_UseTextures == 0)
			alpha = v_Color.a;
		else
			alpha = texture(u_AlbedoTexture, v_TexCoord).a * v_Color.a;
	}

	if(alpha < 0.004)
		discard;


	//Resulting Color
	if(u_UseTextures == 0 || (u_UseTextures == 1 && u_HasTransparencies == 0 && texture(u_AlbedoTexture, v_TexCoord).a < 0.1))
		out_color = vec4(v_Color.rgb, alpha);
	else if(u_UseTextures == 1)
		out_color = vec4(v_Color.rgb * texture(u_AlbedoTexture, v_TexCoord).rgb, alpha);

	out_color = pow(out_color, vec4(vec3(1.0/u_GammaCorrection), 1.0));
}

#endif //FRAGMENT_SHADER
