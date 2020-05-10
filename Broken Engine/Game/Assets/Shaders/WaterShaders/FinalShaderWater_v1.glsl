#version 440 core

#define VERTEX_SHADER
#ifdef VERTEX_SHADER

//Layouts
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec3 a_Color;
layout (location = 3) in vec2 a_TexCoord;

//Uniforms
uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Proj;

uniform float time = 0.0;
uniform float Amplitude = 5.0;
uniform vec2 FOAMDirection = vec2(0, 0);
uniform float FOAMVelocity = 0.5;
uniform float MaxTime = 10.0;
uniform float Velocity = 10.0;
uniform float WaveLength = 1.0;
uniform float WaveMovementMultiplicator = 2.0;

//Variables
const float pi = 3.14159;

//Data sent to fragment shader (varying)
out float v_VertHeight;
out float v_MaxHeight;
out vec2 v_TexCoords;

float random(vec2 p)
{
     vec2 K1 = vec2(23.14069263277926, 2.665144142690225);

   //return fract( cos( mod( 12345678., 256. * dot(p,K1) ) ) ); // ver1
     return fract(cos(dot(p,K1)) * 12345.6789); // ver3
}

float random2(vec2 co)
{
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

void main()
{
	//Water Calcs.
	float height = Amplitude*sin(2.0*pi*((time/MaxTime)*Velocity - (a_Position.x/WaveLength)));
	float dX = sin(2.0*a_Position.x + time);
	float dZ = sin(2.0*a_Position.y + time);
	//float Xf = a_Position.x/u_WaveLength - 0.5;
	//float Yf = a_Position.y/u_Amplitude - 0.5;
	//
	//vec2 vec = vec2(Xf, Yf);
	//float height = random2(vec) * 2.0*pi*(u_Time/u_MaxTime);
	vec2 tCoordsDir = clamp(normalize(FOAMDirection), 0.0, 1.0);

	vec2 tCoords = a_TexCoord;
	tCoords.x -= FOAMVelocity * time/MaxTime * tCoordsDir.x;
	tCoords.y -= FOAMVelocity * time/MaxTime * tCoordsDir.y;

	v_VertHeight = height;
	v_MaxHeight = Amplitude;	
	v_TexCoords = tCoords;

	gl_Position = u_Proj * u_View * u_Model * vec4(a_Position.x, a_Position.y, a_Position.z + dX*dZ*WaveMovementMultiplicator, 1.0);
	gl_Position.y += height;
}


#endif //VERTEX_SHADER

#define FRAGMENT_SHADER
#ifdef FRAGMENT_SHADER

//Uniforms
uniform sampler2D u_AlbedoTexture;
uniform sampler2D u_SpecularTexture;

uniform int u_TextureEmpty = 1;
//uniform int u_UseTextures = 0;
//uniform int u_HasDiffuseTexture = 0;

//Uniforms
uniform float u_GammaCorrection = 1.0;
uniform vec4 u_AmbientColor = vec4(1.0);
uniform vec4 u_Color = vec4(1.0);
uniform float u_ColorGradingOffset = 0.0; //Keep it between 0 and 1

//Data sent from vertex shader
in float v_VertHeight;
in float v_MaxHeight;
in vec2 v_TexCoords;

//Color output
out vec4 color;

void main()
{
	float normalisedHeight = v_VertHeight/(v_MaxHeight + 2.0);
	float colorGrading = u_ColorGradingOffset;

	if(colorGrading >= 1.0)
		colorGrading = 1.0;
	if(colorGrading <= 0.0)
		colorGrading = 0.0;

	color = texture(u_AlbedoTexture, v_TexCoords) * u_AmbientColor * u_Color;// + texture(u_SpecularTexture, v_TexCoords) * 0.2;
	//color.r *= (colorGrading + normalisedHeight);
	//color.g *= (colorGrading + normalisedHeight);

	color = pow(color, vec4(vec3(1.0/u_GammaCorrection), 1.0));
}

#endif