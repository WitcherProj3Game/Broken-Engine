#version 450 core
#define VERTEX_SHADER
#ifdef VERTEX_SHADER

layout (location = 0) in vec3 a_Position;			
uniform mat4 u_Proj;
uniform mat4 u_View;
uniform mat4 u_Model;			
void main()
{
	//gl_Position = u_Proj * u_View * u_Model * vec4(a_Position, 1.0);
	gl_Position =  u_Model * vec4(a_Position, 1.0);

} 
#endif
#define FRAGMENT_SHADER
#ifdef FRAGMENT_SHADER		

in vec4 FragPos;

uniform vec3 lightPos;
uniform float far_plane;

void main()
{
    // get distance between fragment and light source
    float lightDistance = length(FragPos.xyz - vec3(0,1,0));
    
    // map to [0;1] range by dividing by far_plane
    lightDistance = lightDistance / 20; // far_plane
    
    // write this as modified depth
    gl_FragDepth = lightDistance;
}
#endif

#define GEOMETRY_SHADER
#ifdef GEOMETRY_SHADER

layout (triangles) in; 
layout (triangle_strip, max_vertices = 18) out; 

uniform mat4 shadowMatrices[6];

out vec4 FragPos; // FragPos from GS (output per emitvertex)

void main()
{ 
	for (int face = 0; face < 6; face++) 
	{
		gl_Layer = face; // built-in variable that specifies to which face we render.
        for(int i = 0; i < 3; ++i) // for each triangle vertex
        {
            FragPos = gl_in[i].gl_Position;
            gl_Position = shadowMatrices[face] * FragPos;
            EmitVertex();
        }    
        EndPrimitive();
	}
} 
	
#endif //GEOMETRY_SHADER