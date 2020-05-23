#version 450 core
			#define VERTEX_SHADER
			#ifdef VERTEX_SHADER
			
			layout (location = 0) in vec3 a_Position;			
			uniform mat4 u_Proj;
			uniform mat4 u_View;
			uniform mat4 u_Model;			
			void main()
			{
				gl_Position = u_Proj * u_View * u_Model * vec4(a_Position, 1.0);
			} 
			#endif
#define FRAGMENT_SHADER
			#ifdef FRAGMENT_SHADER			
			void main()
			{ 
			}
			#endif