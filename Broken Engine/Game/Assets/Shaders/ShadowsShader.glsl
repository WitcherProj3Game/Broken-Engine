#version 450 core
#define VERTEX_SHADER
#ifdef VERTEX_SHADER

layout (location = 0) in vec3 a_Position;	
layout (location = 3) in vec2 a_TexCoord;

uniform mat4 u_Proj;
uniform mat4 u_View;
uniform mat4 u_Model;

out vec2 v_texCoords;

void main()
{
	v_texCoords = a_TexCoord;
	gl_Position = u_Proj * u_View * u_Model * vec4(a_Position, 1.0);
}

#endif

#define FRAGMENT_SHADER
#ifdef FRAGMENT_SHADER

uniform sampler2D u_AlbedoTexture;
uniform int u_HasDiffuseTexture = 0;
uniform vec4 u_Color = vec4(1.0);

in vec2 v_texCoords;

void main()
{
	vec4 color = vec4(1.0);
	if(u_HasDiffuseTexture == 1)
		color = texture(u_AlbedoTexture, v_texCoords) * u_Color;
	else
		color = u_Color;

	if(color.a < 0.01)
		discard;
}

#endif