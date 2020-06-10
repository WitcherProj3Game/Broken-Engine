#version 440 core 
#define VERTEX_SHADER 
#ifdef VERTEX_SHADER 
layout (location = 0) in vec3 a_Position;
uniform mat4 u_Model; 
uniform mat4 u_View; 
uniform mat4 u_Proj; 

uniform vec4 u_Color = vec4(0,0,0,1); // Color of outline
out vec4 v_Color;
void main()
{
    v_Color = u_Color;

    vec3 position = vec3(u_Model * vec4(a_Position, 1.0));
    gl_Position = u_Proj * u_View * vec4(position, 1.0f); 
}
#endif //VERTEX_SHADER

#define FRAGMENT_SHADER 
#ifdef FRAGMENT_SHADER 

in vec4 v_Color;
out vec4 color; 
void main()
{ 
    color = v_Color;
} 
#endif //FRAGMENT_SHADER 
