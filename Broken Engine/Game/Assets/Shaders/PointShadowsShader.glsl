#version 440 core
#define VERTEX_SHADER
#ifdef VERTEX_SHADER			

layout (location = 0) in vec3 a_Position;	
layout (location = 3) in vec2 a_TexCoord;			

uniform mat4 u_Proj;
uniform mat4 u_View;
uniform mat4 u_Model;			

//out vData
//{
//	vec2 v_texCoords;
//}vertex;			

void main()
{
	//vertex.v_texCoords = a_TexCoord;
	gl_Position = u_Proj * u_View * u_Model * vec4(a_Position, 1.0);
}		

#endif

#define FRAGMENT_SHADER
#ifdef FRAGMENT_SHADER
uniform sampler2D u_AlbedoTexture;
uniform int u_HasDiffuseTexture = 0;
uniform vec4 u_Color = vec4(1.0);
uniform vec3 lightPos = vec3(0.0, 5.0, 0.0);

in vec4 FragPos;


//in fData
//{
//	vec2 v_texCoords;
//}frag;

void main()
{
	//vec4 color = vec4(1.0);
	//if(u_HasDiffuseTexture == 1)
	//	color = texture(u_AlbedoTexture, frag.v_texCoords) * u_Color;
	//else
	//	color = u_Color;
	//
	//if(color.a < 0.01)
	//	discard;

	float lightDistance = length(FragPos.xyz - lightPos);
    
    // map to [0;1] range by dividing by far_plane
    lightDistance = lightDistance / 1000.0;
    
    // write this as modified depth
    gl_FragDepth = lightDistance;

}
#endif

#define GEOMETRY_SHADER
#ifdef GEOMETRY_SHADER

layout (triangles) in; 
layout (triangle_strip, max_vertices = 18) out; 	

uniform mat4 shadowMatrices[6];	

//in vData
//{
//	vec2 v_texCoords;
//}vertex[];			

//out fData
//{
//	vec2 v_texCoords;
//}frag;			

out vec4 FragPos; // FragPos from GS (output per emitvertex)	

void main()
{ 
	for (int face = 0; face < 6; ++face) 
	{
		gl_Layer = face; // built-in variable that specifies to which face we render.
        for(int i = 0; i < 3; ++i) // for each triangle vertex
        {
			//frag.v_texCoords = vertex[face*3 + i].v_texCoords;
            FragPos = gl_in[i].gl_Position;
			//gl_Position = gl_in[i].gl_Position;
            gl_Position = shadowMatrices[face] * FragPos;
            EmitVertex();
        }    
        EndPrimitive();
	}
}				
#endif //GEOMETRY_SHADER