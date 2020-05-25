#version 440 core

#define VERTEX_SHADER
#ifdef VERTEX_SHADER

//Layout Daya
layout (location = 0) in vec3 a_Position;
layout (location = 1) in vec3 a_Normal;
layout (location = 2) in vec3 a_Color;
layout (location = 3) in vec2 a_TexCoord;
layout (location = 4) in vec3 a_Tangent;
layout (location = 5) in vec3 a_Bitangent;

//Uniforms
uniform mat4 u_Model; //model_matrix
uniform mat4 u_View; //view
uniform mat4 u_Proj; //projection
uniform mat4 u_LightSpace;

uniform vec4 u_Color = vec4(1.0); //Color
uniform vec3 u_CameraPosition;

//Varyings

out vData
{
	vec2 v_TexCoord;
	vec4 v_Color;
	vec3 v_Normal;
	vec3 v_FragPos;
	vec3 v_CamPos;
	mat3 v_TBN;
	vec4 v_FragPos_InLightSpace;
}vertex;

void main()
{
	vertex.v_Color = u_Color;
	vertex.v_TexCoord = a_TexCoord;
	vertex.v_CamPos = u_CameraPosition;

	vertex.v_FragPos = vec3(u_Model * vec4(a_Position, 1.0));
	vertex.v_FragPos_InLightSpace = u_LightSpace * vec4(vertex.v_FragPos, 1.0);

	vertex.v_Normal = transpose(inverse(mat3(u_Model))) * a_Normal;

	vec3 T = normalize(vec3(u_Model * vec4(a_Tangent, 0.0)));
	vec3 N = normalize(vec3(u_Model * vec4(a_Normal, 0.0)));
	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(N, T);
	vertex.v_TBN = (mat3(T, B, N));

	//gl_Position = u_Proj * u_View * u_Model * vec4(a_Position, 1.0);
	gl_Position = u_Proj * u_View * vec4(vertex.v_FragPos, 1.0);
}

#endif //VERTEX_SHADER
#define FRAGMENT_SHADER
#ifdef FRAGMENT_SHADER

#define MAX_SHADER_LIGHTS 20

//Output Variables
out vec4 out_color;

//Input Variables (Varying)
in fData
{
	vec2 f_TexCoord;
	vec4 f_Color;
	vec3 f_Normal;
	vec3 f_FragPos;
	vec3 f_CamPos;
	mat3 f_TBN;
	vec4 f_FragPos_InLightSpace;
}frag;

//Uniforms
uniform float u_GammaCorrection = 1.0;
uniform vec4 u_AmbientColor = vec4(1.0);

uniform float u_Shininess = 1.5;
uniform int u_UseTextures = 0;

uniform int u_HasDiffuseTexture = 0;
uniform int u_HasSpecularTexture = 0;
uniform int u_HasNormalMap = 0;
uniform int u_HasTransparencies = 0;

uniform int u_DrawNormalMapping = 0;
uniform int u_DrawNormalMapping_Lit = 0;
uniform int u_DrawNormalMapping_Lit_Adv = 0;

uniform sampler2D u_AlbedoTexture;
uniform sampler2D u_SpecularTexture;
uniform sampler2D u_NormalTexture;
uniform sampler2D u_ShadowMap;
uniform samplerCube depthMap;

//Shadows Uniforms
uniform bool u_ReceiveShadows = true;
uniform float u_ShadowIntensity = 1.0;
uniform float u_ShadowBias = 0.001;

uniform float u_ShadowPoissonBlur = 700.0;
uniform float u_ShadowOffsetBlur = 0.2;
uniform float u_ShadowPCFDivisor = 9.0;
uniform bool u_ShadowSmootherPCF = false;
uniform bool u_ShadowSmootherPoissonDisk = true;
uniform bool u_ShadowSmootherBoth = false;
uniform bool u_ClampShadows = false;
uniform float u_ShadowsSmoothMultiplicator = 1.0;

//Other Variables
vec2 poissonDisk[4] = vec2[](vec2(-0.94201624, -0.39906216 ), vec2(0.94558609, -0.76890725), vec2(-0.094184101, -0.92938870 ), vec2(0.34495938, 0.29387760));

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
	bool LightCastingShadows;
};

uniform int u_LightsNumber = 0;
uniform BrokenLight u_BkLights[MAX_SHADER_LIGHTS];
// uniform BrokenLight u_BkLights[MAX_SHADER_LIGHTS] = BrokenLight[MAX_SHADER_LIGHTS](BrokenLight(vec3(0.0), vec3(0.0), vec3(1.0), 0.5, vec3(1.0, 0.09, 0.032), vec2(12.5, 45.0), 2));

//Light Calculations Functions ---------------------------------------------------------------------------------------
//Shadows Calculation
float ShadowCalculation(vec3 dir, vec3 normal)
{
	vec3 projCoords = frag.f_FragPos_InLightSpace.xyz / frag.f_FragPos_InLightSpace.w;
	projCoords = projCoords * 0.5 + 0.5;

	float currDept = projCoords.z;
	float bias = max(0.01 * (1.0 - dot(normal, dir)), u_ShadowBias);
	
	float shadow = 0.0;
	if(u_ShadowSmootherPCF || u_ShadowSmootherBoth)
	{
		vec2 texelSize = 1.0/textureSize(u_ShadowMap, 0);
		for(int x = -1; x <= 1; ++x)
		{
			for(int y = -1; y <= 1; ++y)
			{
				float pcfDepth = texture(u_ShadowMap, projCoords.xy + vec2(x,y)*texelSize).r;
				shadow += currDept - bias > pcfDepth ? 1.0 : 0.0;
			}
		}
		shadow /= u_ShadowPCFDivisor;
	}

	if(u_ShadowSmootherPoissonDisk || u_ShadowSmootherBoth)
	{
		if(!u_ShadowSmootherBoth)
			shadow = ((currDept - bias) > texture(u_ShadowMap, projCoords.xy).z ? 1.0 : 0.0);

		for(int i = 0; i < 4; ++i)
			if(texture(u_ShadowMap, projCoords.xy+poissonDisk[i]/u_ShadowPoissonBlur).z <  currDept - bias)
				shadow -= u_ShadowOffsetBlur;
	}

	if(projCoords.z > 1.0)
		shadow = 0.0;

	if(u_ClampShadows)
		shadow = clamp(shadow, 0.0, 3.0);
	else if(shadow > 1.0 || shadow < 0.0)
		shadow *= u_ShadowsSmoothMultiplicator;

	return (shadow * u_ShadowIntensity);
}

float OmnidirectionalShadowCalculation(vec3 lightPos)
{
    // get vector between fragment position and light position
    vec3 fragToLight = frag.f_FragPos - vec3(0,1,0); // should be lightPos
    // use the light to fragment vector to sample from the depth map    
    float closestDepth = texture(depthMap, fragToLight).r;
    // it is currently in linear range between [0,1]. Re-transform back to original value
    closestDepth *= 20; // should be far_plane
    // now get current linear depth as the length between the fragment and light position
    float currentDepth = length(fragToLight);
    // now test for shadows
    float bias = 0.05; 
    float shadow = currentDepth -  bias > closestDepth ? 1.0 : 0.0;

    return shadow;
}

//Light Calculations Functions ---------------------------------------------------------------------------------------
vec3 CalculateLightResult(vec3 LColor, vec3 LDir, vec3 normal, vec3 viewDir, bool lightShadower, vec3 pos)
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
		diffuse *= texture(u_AlbedoTexture, frag.f_TexCoord).rgb;
	if(u_HasSpecularTexture == 1)
		specular *= texture(u_SpecularTexture, frag.f_TexCoord).rgb;

	if(u_ReceiveShadows && lightShadower)
	{
		//return (1.0 - ShadowCalculation(lightDir, normal)) * (diffuse + specular);

		return (1.0 - OmnidirectionalShadowCalculation(pos)) * (diffuse + specular);
	}
	return (1.0 - OmnidirectionalShadowCalculation(pos)) * (diffuse + specular);

	return (diffuse + specular);
}


//Dir Light Calculation
vec3 CalculateDirectionalLight(BrokenLight light, vec3 normal, vec3 viewDir)
{
	if(u_HasNormalMap == 1)
		return CalculateLightResult(light.color, /*v_TBN * */normalize(light.dir), normal, viewDir, light.LightCastingShadows, vec3(0,0,0)) * light.intensity;
	else
		return CalculateLightResult(light.color, light.dir, normal, viewDir, light.LightCastingShadows, vec3(0,0,0)) * light.intensity;
}

//Point Light Calculation
vec3 CalculatePointlight(BrokenLight light, vec3 normal, vec3 viewDir)
{
	//Calculate light direction
	vec3 direction = light.pos - frag.f_FragPos;
	//if(u_HasNormalMap == 1)
	//	direction = v_TBN * normalize(direction);

	//Attenuation Calculation
	float dMult = 1/light.distanceMultiplier;
	float d = length(light.pos - frag.f_FragPos) * dMult;
	float lightAttenuation = 1.0/(light.attenuationKLQ.x + light.attenuationKLQ.y * d + light.attenuationKLQ.z *(d * d));

	//Result
	return CalculateLightResult(light.color, direction, normal, viewDir, false, light.pos) * lightAttenuation * light.intensity;
}

//Spot Light Calculation
vec3 CalculateSpotlight(BrokenLight light, vec3 normal, vec3 viewDir)
{
	//Calculate light direction
	vec3 direction = light.pos - frag.f_FragPos;
	//if(u_HasNormalMap == 1)
	//	direction = v_TBN * normalize(direction);

	//Attenuation Calculation
	float d = length(light.pos - frag.f_FragPos);
	float lightAttenuation = 1.0/ (light.attenuationKLQ.x + light.attenuationKLQ.y * d + light.attenuationKLQ.z *(d * d));

	//Spotlight Calcs for Soft Edges
	float theta = dot(normalize(light.pos - frag.f_FragPos), normalize(-light.dir)); //Light direction and light orientation
	float epsilon = light.InOutCutoff.x - light.InOutCutoff.y;

	float lightIntensity = clamp((theta - light.InOutCutoff.y) / epsilon, 0.0, 1.0) * light.intensity;

	//Result
	return CalculateLightResult(light.color, direction, normal, viewDir, false, vec3(0,0,0)) * lightAttenuation * lightIntensity;
}

//------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------
void main()
{
	// Transparency
	float alpha = 1.0;
	if(u_HasTransparencies == 1)
	{
		if(u_UseTextures == 0)
			alpha = frag.f_Color.a;
		else
			alpha = texture(u_AlbedoTexture, frag.f_TexCoord).a * frag.f_Color.a;
	}

	if(alpha < 0.004)
		discard;

	//Normal Mapping Calculations
	vec3 normalVec = normalize(frag.f_Normal);	
	if(u_DrawNormalMapping == 1)
	{
		out_color = vec4(normalVec, 1.0);
		return;
	}
	
	vec3 viewDirection = normalize(frag.f_CamPos - frag.f_FragPos);
	if(u_HasNormalMap == 1)
	{
		normalVec = texture(u_NormalTexture, frag.f_TexCoord).rgb;
		normalVec = normalize(normalVec * 2.0 - 1.0);
		normalVec = normalize(frag.f_TBN * normalVec);
		//viewDirection = v_TBN * normalize(v_CamPos - v_FragPos);
	}

	//Light Calculations
	int lights_iterator = (u_LightsNumber > MAX_SHADER_LIGHTS ? MAX_SHADER_LIGHTS : u_LightsNumber);
	vec3 colorResult = vec3(0.0);
	for(int i = 0; i < lights_iterator; ++i)
	{
		if(u_DrawNormalMapping_Lit_Adv == 0)
		{
			if(u_BkLights[i].LightType == 0) //Directional
				colorResult += CalculateDirectionalLight(u_BkLights[i], normalVec, viewDirection);

			else if(u_BkLights[i].LightType == 1) //Pointlight
				colorResult += CalculatePointlight(u_BkLights[i], normalVec, viewDirection);

			else if(u_BkLights[i].LightType == 2) //Spotlight
				colorResult += CalculateSpotlight(u_BkLights[i], normalVec, viewDirection);
		}
		else
		{
			if(u_BkLights[i].LightType == 0)
				colorResult += frag.f_TBN * normalize(u_BkLights[i].dir);
			else
				colorResult += frag.f_TBN * normalize(u_BkLights[i].pos);
		}
	}

	if(u_DrawNormalMapping_Lit == 0 && u_DrawNormalMapping_Lit_Adv == 0)
	{
		vec3 finalColor = u_AmbientColor.rgb * frag.f_Color.rgb;

		//Resulting Color
		if(u_UseTextures == 0 || (u_HasTransparencies == 0 && u_UseTextures == 1 && texture(u_AlbedoTexture, frag.f_TexCoord).a < 0.1))
			out_color = vec4(colorResult + finalColor, alpha);
		else if(u_UseTextures == 1)
			out_color = vec4(colorResult + finalColor * texture(u_AlbedoTexture, frag.f_TexCoord).rgb, alpha);

		out_color = pow(out_color, vec4(vec3(1.0/u_GammaCorrection), 1.0));
	}
	else
	{
		//Normal Mapping Debgug
		if(u_DrawNormalMapping_Lit == 1)
			out_color = vec4(colorResult * normalVec, 1.0);
		else if(u_DrawNormalMapping_Lit_Adv == 1)
			out_color = vec4(colorResult, 1.0);
	}
}

#endif //FRAGMENT_SHADER

#define GEOMETRY_SHADER
#ifdef GEOMETRY_SHADER

layout (triangles) in; 
layout (triangle_strip, max_vertices = 3) out; 

in vData
{
	vec2 v_TexCoord;
	vec4 v_Color;
	vec3 v_Normal;
	vec3 v_FragPos;
	vec3 v_CamPos;
	mat3 v_TBN;
	vec4 v_FragPos_InLightSpace;
}vertex[];

out fData
{
	vec2 f_TexCoord;
	vec4 f_Color;
	vec3 f_Normal;
	vec3 f_FragPos;
	vec3 f_CamPos;
	mat3 f_TBN;
	vec4 f_FragPos_InLightSpace;
}frag;

void main()
{ 
	for (int i = 0; i < gl_in.length(); i++) 
	{
		gl_Position = gl_in[i].gl_Position;
		frag.f_TexCoord = vertex[i].v_TexCoord;
		frag.f_Color = vertex[i].v_Color;
		frag.f_Normal = vertex[i].v_Normal;
		frag.f_FragPos = vertex[i].v_FragPos;
		frag.f_CamPos = vertex[i].v_CamPos;
		frag.f_TBN = vertex[i].v_TBN;
		frag.f_FragPos_InLightSpace = vertex[i].v_FragPos_InLightSpace;

		EmitVertex();
	}

	EndPrimitive();
} 
	
#endif //GEOMETRY_SHADER



