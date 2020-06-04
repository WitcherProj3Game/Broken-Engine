#version 460 core 
#define VERTEX_SHADER 
#ifdef VERTEX_SHADER 

layout (location = 0) in vec3 a_Position; 

uniform mat4 u_View; 
uniform mat4 u_Proj; 
uniform mat4 u_Model; 

out vec3 TexCoords; 

void main()
{ 
	TexCoords = a_Position * vec3(1.0, -1.0, 1.0); 
	gl_Position = u_Proj * u_View * u_Model * vec4(a_Position, 1.0); 
}
#endif //VERTEX_SHADER

#define FRAGMENT_SHADER 
#ifdef FRAGMENT_SHADER 

in vec3 TexCoords; 

uniform float u_Exposure = 1.0;
uniform vec3 u_Color = vec3(1.0);
uniform float u_GammaCorrection = 1.0;
uniform samplerCube skybox;

out vec4 color; 

void main()
{
	vec4 tex_color = texture(skybox, TexCoords) * vec4(u_Color, 1.0) * u_Exposure;
	color = pow(tex_color, vec4(vec3(1.0/u_GammaCorrection), 1.0));
} 
#endif //FRAGMENT_SHADER 
