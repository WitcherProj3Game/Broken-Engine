#version 440 core
#define VERTEX_SHADER
#ifdef VERTEX_SHADER

layout (location = 0) in vec2 aPos;

void main()
{
	gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0); 
} 
#endif //VERTEX_SHADER

#define FRAGMENT_SHADER
#ifdef FRAGMENT_SHADER

out vec4 FragColor;
  
uniform sampler2D screenTexture;
uniform vec2 screenTexture_size;

void main()
{ 
    FragColor = texture(screenTexture, gl_FragCoord.xy / screenTexture_size);
}
#endif //FRAGMENT_SHADER