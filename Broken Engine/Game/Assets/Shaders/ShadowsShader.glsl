#version 450 core
#define VERTEX_SHADER
#ifdef VERTEX_SHADER

layout (location = 0) in vec3 a_Position;	
layout (location = 3) in vec2 a_TexCoord;

uniform mat4 u_Proj;
uniform mat4 u_View;
uniform mat4 u_Model;

out vec2 vg_texCoords;

void main()
{
	vg_texCoords = a_TexCoord;
	gl_Position = u_Proj * u_View * u_Model * vec4(a_Position, 1.0);
}

#endif

#define FRAGMENT_SHADER
#ifdef FRAGMENT_SHADER

uniform sampler2D u_AlbedoTexture;
uniform int u_HasDiffuseTexture = 0;
uniform vec4 u_Color = vec4(1.0);

in vec4 FragPos;

// FOR point shadows

//uniform vec3 lightPos;
//uniform float far_plane;

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


	// FOR point shadows

    // get distance between fragment and light source
    //float lightDistance = length(FragPos.xyz - vec3(0,1,0));
    
    // map to [0;1] range by dividing by far_plane
    //lightDistance = lightDistance / 20; // far_plane
    
    // write this as modified depth
    //gl_FragDepth = lightDistance;
}

#endif

#define GEOMETRY_SHADER
#ifdef GEOMETRY_SHADER

layout (triangles) in; 
layout (triangle_strip, max_vertices = 18) out; 

uniform mat4 shadowMatrices[6];

in vec2 vg_texCoords[];
out vec2 v_texCoords;

out vec4 FragPos; // FragPos from GS (output per emitvertex)

void main()
{ 
	for (int face = 0; face < 6; face++) 
	{
		gl_Layer = face; // built-in variable that specifies to which face we render.
        for(int i = 0; i < 3; ++i) // for each triangle vertex
        {
			v_texCoords = vg_texCoords[face*3 + i];
            FragPos = gl_in[i].gl_Position;
            gl_Position = shadowMatrices[face] * FragPos;
            EmitVertex();
        }    
        EndPrimitive();
	}
} 
	
#endif //GEOMETRY_SHADER