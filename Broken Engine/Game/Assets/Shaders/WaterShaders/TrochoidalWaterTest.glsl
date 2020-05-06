#version 440 core

#define VERTEX_SHADER
#ifdef VERTEX_SHADER

//Layouts
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec3 a_Color;
layout (location = 3) in vec2 a_TexCoord;
layout (location = 4) in vec3 a_Tangent;
layout (location = 5) in vec3 a_Bitangent;

// Common Uniforms
uniform mat4 u_Model;
uniform mat4 u_View;
uniform mat4 u_Proj;
uniform vec3 u_CameraPosition;

// Specific Uniforms
uniform float time = 0.0;
uniform float MaxTime = 10.0;
uniform float Amplitude = 5.0;
uniform float WaveLength = 1.0;
uniform float Velocity = 10.0;

//Variables
const float pi = 3.14159;

//Varyings
out vec2 v_TexCoords;
out vec3 v_Normal;
out vec3 v_CamPos;
out vec3 v_FragPos;
out float v_MaxHeight;
out mat3 v_TBN;

void main()
{
	//Simple Varyings
	v_MaxHeight = Amplitude;
	v_TexCoords = a_TexCoord;
	v_CamPos = u_CameraPosition;

	//Varyings Calcs
	v_FragPos = vec3(u_Model * vec4(a_Position, 1.0));
	v_Normal = mat3(transpose(inverse(u_Model))) * a_Normal;

	//Vertex Positioning
	vec3 pos = a_Position;
	float k = 2*pi/WaveLength;
	float f = k * (pos.x - Velocity*time);

	vec3 tangent = normalize(vec3(1.0, k*Amplitude*cos(f), 0.0));
	vec3 normalV = vec3(-tangent.y, tangent.x, 0);
	v_Normal = normalV;

	pos.z += Amplitude * sin(f);
	gl_Position = u_Proj * u_View * u_Model * vec4(pos, 1.0);


	//Normal Matrix
	//vec3 T = normalize(vec3(u_Model * vec4(a_Tangent, 0.0)));
	//vec3 N = normalize(vec3(u_Model * vec4(a_Normal, 0.0)));
	vec3 T = normalize(vec3(u_Model * vec4(tangent, 0.0)));
	vec3 N = normalize(vec3(u_Model * vec4(v_Normal, 0.0)));
	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(N, T);
	v_TBN = (mat3(T, B, N));
}


#endif //VERTEX_SHADER

#define FRAGMENT_SHADER
#ifdef FRAGMENT_SHADER

#define MAX_SHADER_LIGHTS 60

//Light Uniforms
struct BrokenLight
{
	vec3 dir;
	vec3 pos;
	vec3 color;

	float intensity;
	float distanceMultiplier;

	vec3 attenuationKLQ;
	vec2 InOutCutoff;

	int LightType;
};

uniform int u_LightsNumber = 0;
uniform BrokenLight u_BkLights[MAX_SHADER_LIGHTS];

//Uniforms
uniform sampler2D u_AlbedoTexture;
uniform sampler2D u_SpecularTexture;
uniform sampler2D u_NormalTexture;
uniform samplerCube skybox;

uniform int u_TextureEmpty = 1;

// Material Uniforms
uniform float u_Shininess = 1.5;

uniform int u_HasNormalMap = 0;
uniform int u_HasDiffuseTexture = 0;
uniform int u_HasSpecularTexture = 0;

//Uniforms
uniform float u_GammaCorrection = 1.0;
uniform vec4 u_AmbientColor = vec4(1.0);
uniform vec4 u_Color = vec4(1.0);

uniform int None_Reflect_Refract = 0;
uniform float refractive_reflective_index = 1.0;

//Data sent from vertex shader
in vec2 v_TexCoords;
in vec3 v_Normal;
in vec3 v_CamPos;
in vec3 v_FragPos;
in float v_MaxHeight;
in mat3 v_TBN;

//Color output
out vec4 color;

vec3 CalculateLightResult(vec3 LColor, vec3 LDir, vec3 normal, vec3 viewDir);
vec3 CalculateDirectionalLight(BrokenLight light, vec3 normal, vec3 viewDir);
vec3 CalculateSpotlight(BrokenLight light, vec3 normal, vec3 viewDir);
vec3 CalculatePointlight(BrokenLight light, vec3 normal, vec3 viewDir);
vec4 CalculateRefractionAndReflection();

void main()
{
	//Normal Mapping Calculations
	vec3 normalVec = normalize(v_Normal);	
	vec3 viewDirection = normalize(v_CamPos - v_FragPos);
	if(u_HasNormalMap == 1)
	{
		normalVec = texture(u_NormalTexture, v_TexCoords).rgb;
		normalVec = normalize(normalVec * 2.0 - 1.0);
		normalVec = normalize(v_TBN * normalVec);
		//viewDirection = v_TBN * normalize(v_CamPos - v_FragPos);
	}

	//Light Calculations
	vec3 colorResult = vec3(0.0);
	int lights_iterator = (u_LightsNumber > MAX_SHADER_LIGHTS ? MAX_SHADER_LIGHTS : u_LightsNumber);
	for(int i = 0; i < lights_iterator; ++i)
	{
		if(u_BkLights[i].LightType == 0) //Directional
			colorResult += CalculateDirectionalLight(u_BkLights[i], normalVec, viewDirection);

		else if(u_BkLights[i].LightType == 1) //Pointlight
			colorResult += CalculatePointlight(u_BkLights[i], normalVec, viewDirection);

		else if(u_BkLights[i].LightType == 2) //Spotlight
			colorResult += CalculateSpotlight(u_BkLights[i], normalVec, viewDirection);
	}

	//Final Color
	vec4 reflectingColor = CalculateRefractionAndReflection();
	color = vec4(colorResult, 1.0) + texture(u_AlbedoTexture, v_TexCoords) * u_AmbientColor * u_Color;
	color += reflectingColor;
	color = pow(color, vec4(vec3(1.0/u_GammaCorrection), 1.0));
}


vec4 CalculateRefractionAndReflection()
{
	if(None_Reflect_Refract == 1)
	{
		vec3 I = normalize(v_FragPos - v_CamPos);
		vec3 R = reflect(I, normalize(v_Normal));
		return vec4(texture(skybox, R).rgb, 1.0) * refractive_reflective_index;
	}
	else if(None_Reflect_Refract == 2)
	{
		float ratio = 1.0/refractive_reflective_index;
		vec3 I = normalize (v_FragPos - v_CamPos);
		vec3 R = refract(I, normalize(v_Normal), ratio);
		return vec4(texture(skybox, R).rgb, 1.0);
	}
	else
		return vec4(0.0);
}


//Light Calculations Functions ---------------------------------------------------------------------------------------
vec3 CalculateLightResult(vec3 LColor, vec3 LDir, vec3 normal, vec3 viewDir)
{
	//Normalize light direction
	vec3 lightDir = normalize(LDir);

	//Diffuse Component
	float diffImpact = max(dot(normal, lightDir), 0.0);

	//Specular component
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float specImpact = pow(max(dot(normal, halfwayDir), 0.0), u_Shininess);

	//Calculate light result
	vec3 diffuse = LColor * diffImpact;
	vec3 specular = LColor * specImpact;

	//If we have textures, apply them
	if(u_HasDiffuseTexture == 1)
		diffuse *= texture(u_AlbedoTexture, v_TexCoords).rgb;
	if(u_HasSpecularTexture == 1)
		specular *= texture(u_SpecularTexture, v_TexCoords).rgb;

	return (diffuse + specular);
}

//Dir Light Calculation
vec3 CalculateDirectionalLight(BrokenLight light, vec3 normal, vec3 viewDir)
{
	if(u_HasNormalMap == 1)
		return CalculateLightResult(light.color, /*v_TBN * */normalize(light.dir), normal, viewDir) * light.intensity;
	else
		return CalculateLightResult(light.color, light.dir, normal, viewDir) * light.intensity;
}

//Point Light Calculation
vec3 CalculatePointlight(BrokenLight light, vec3 normal, vec3 viewDir)
{
	//Calculate light direction
	vec3 direction = light.pos - v_FragPos;
	//if(u_HasNormalMap == 1)
	//	direction = v_TBN * normalize(direction);

	//Attenuation Calculation
	float dMult = 1/light.distanceMultiplier;
	float d = length(light.pos - v_FragPos) * dMult;
	float lightAttenuation = 1.0/(light.attenuationKLQ.x + light.attenuationKLQ.y * d + light.attenuationKLQ.z *(d * d));

	//Result
	return CalculateLightResult(light.color, direction, normal, viewDir) * lightAttenuation * light.intensity;
}

//Spot Light Calculation
vec3 CalculateSpotlight(BrokenLight light, vec3 normal, vec3 viewDir)
{
	//Calculate light direction
	vec3 direction = light.pos - v_FragPos;
	//if(u_HasNormalMap == 1)
	//	direction = v_TBN * normalize(direction);

	//Attenuation Calculation
	float d = length(light.pos - v_FragPos);
	float lightAttenuation = 1.0/ (light.attenuationKLQ.x + light.attenuationKLQ.y * d + light.attenuationKLQ.z *(d * d));

	//Spotlight Calcs for Soft Edges
	float theta = dot(normalize(light.pos - v_FragPos), normalize(-light.dir)); //Light direction and light orientation
	float epsilon = light.InOutCutoff.x - light.InOutCutoff.y;

	float lightIntensity = clamp((theta - light.InOutCutoff.y) / epsilon, 0.0, 1.0) * light.intensity;

	//Result
	return CalculateLightResult(light.color, direction, normal, viewDir) * lightAttenuation * lightIntensity;
}

#endif