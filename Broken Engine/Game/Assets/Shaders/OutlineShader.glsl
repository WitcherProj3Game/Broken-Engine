#version 440 core 
#define VERTEX_SHADER 
#ifdef VERTEX_SHADER 
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 6) in vec3 a_SmoothNormal;
uniform mat4 u_Model; 
uniform mat4 u_View; 
uniform mat4 u_Proj; 
uniform int u_Thickness = 5;
void main()
{
    vec3 newPosition = a_Position + a_SmoothNormal * 2;
    vec3 position = vec3(u_Model * vec4(newPosition, 1.0));
    gl_Position = u_Proj * u_View * vec4(position, 1.0f); 
}
#endif //VERTEX_SHADER

#define FRAGMENT_SHADER 
#ifdef FRAGMENT_SHADER 
out vec4 color; 
void main(){ 
    color = vec4(1.0,0.65,0.0, 1.0); 
} 
#endif //FRAGMENT_SHADER 
