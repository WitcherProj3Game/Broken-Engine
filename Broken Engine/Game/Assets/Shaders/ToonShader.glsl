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
out vec2 v_TexCoord;
out vec4 v_Color;
out vec3 v_Normal;
out vec3 v_FragPos;
out vec3 v_CamPos;
out mat3 v_TBN;
out vec4 v_FragPos_InLightSpace;

void main()
{
	v_Color = u_Color;
	v_TexCoord = a_TexCoord;
	v_CamPos = u_CameraPosition;

	v_FragPos = vec3(u_Model * vec4(a_Position, 1.0));
	v_FragPos_InLightSpace = u_LightSpace * vec4(v_FragPos, 1.0);

	v_Normal = transpose(inverse(mat3(u_Model))) * a_Normal;

	vec3 T = normalize(vec3(u_Model * vec4(a_Tangent, 0.0)));
	vec3 N = normalize(vec3(u_Model * vec4(a_Normal, 0.0)));
	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(N, T);
	v_TBN = (mat3(T, B, N));

	//gl_Position = u_Proj * u_View * u_Model * vec4(a_Position, 1.0);
	gl_Position = u_Proj * u_View * vec4(v_FragPos, 1.0);
}

#endif //VERTEX_SHADER

#define FRAGMENT_SHADER
#ifdef FRAGMENT_SHADER

#define MAX_SHADER_LIGHTS 20

//Output Variables
out vec4 out_color;

//Input Variables (Varying)
in vec2 v_TexCoord;
in vec4 v_Color;
in vec3 v_Normal;
in vec3 v_FragPos;
in vec3 v_CamPos;
in mat3 v_TBN;
in vec4 v_FragPos_InLightSpace;

//Uniforms
uniform float u_GammaCorrection = 1.0;
uniform vec4 u_AmbientColor = vec4(1.0);
uniform bool u_SceneColorAffected = true;
uniform bool u_LightAffected = true;

uniform float u_Shininess = 1.0;
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

//Rim Light Uniforms
uniform bool u_ApplyRimLight = false;
uniform vec2 u_RimSmooth = vec2(0.0, 1.0);
uniform float u_RimPower = 1.0;

//Toon variable
uniform int u_Steps = 4;

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
	vec3 projCoords = v_FragPos_InLightSpace.xyz / v_FragPos_InLightSpace.w;
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

	float toon_ShadowIntensity = u_ShadowIntensity;
	//Toon steps
	float intensity_step = 1.0 / u_Steps;
	for (int i = 0; i < u_Steps; ++i)
	{
		if (toon_ShadowIntensity < intensity_step * (i + 1))
		{
			toon_ShadowIntensity = intensity_step * i;
			break;
		}
	}

	return (shadow * toon_ShadowIntensity);
}

//Light Calculations Functions ---------------------------------------------------------------------------------------
vec3 CalculateRimLight(vec3 normal, vec3 view, vec3 rimColor, float rimPower, vec2 rimSmooth)
{
	float rimFactor = 1.0 - dot(normal, view);
	rimFactor = smoothstep(rimSmooth.x, rimSmooth.y, rimFactor); //Constrain to [0,1] range
	rimFactor = pow(rimFactor, rimPower);

	//Toon steps
	float intensity_step = 1.0 / u_Steps;
	for (int i = 0; i < u_Steps; ++i)
	{
		if (rimFactor < intensity_step * (i + 1))
		{
			rimFactor = intensity_step * i;
			break;
		}
	}

	return rimFactor*rimColor;
}

vec3 CalculateLightResult(vec3 LColor, vec3 LDir, vec3 normal, vec3 viewDir, bool lightShadower)
{
	//Normalize light direction
	vec3 lightDir = normalize(LDir);

	//Diffuse Component
	float diffImpact = max(dot(normal, lightDir), 0.0);

	//Clamp diffuse for toon
	float intensity_step = 1.0 / u_Steps;
	for (int i = 0; i < u_Steps; ++i)
	{
		if (diffImpact < intensity_step * (i + 1))
		{
			diffImpact = intensity_step * i;
			break;
		}
	}

	//Specular component
	vec3 halfwayDir = normalize(lightDir + viewDir);
	float specImpact = pow(max(dot(normal, halfwayDir), 0.0), u_Shininess);

	//Clamp specular for toon
	for (int i = 0; i < u_Steps; ++i)
	{
		if (specImpact < intensity_step * (i + 1))
		{
			specImpact = intensity_step * i;
			break;
		}
	}

	//Calculate light result
	vec3 diffuse = LColor * diffImpact;
	vec3 specular = LColor * specImpact;

	//If we have textures, apply them
	if(u_HasDiffuseTexture == 1)
		diffuse *= texture(u_AlbedoTexture, v_TexCoord).rgb;
	if(u_HasSpecularTexture == 1)
		specular *= texture(u_SpecularTexture, v_TexCoord).rgb;

	vec3 ret = vec3(0.0);
	if(u_LightAffected)
		ret = diffuse + specular;

	if(u_ReceiveShadows && lightShadower)
		ret *= (1.0 - ShadowCalculation(lightDir, normal));

	if(u_ApplyRimLight)
		ret += CalculateRimLight(normal, viewDir, LColor, u_RimPower, u_RimSmooth);

	return ret;
}


//Dir Light Calculation
vec3 CalculateDirectionalLight(BrokenLight light, vec3 normal, vec3 viewDir)
{
		// Calculate toon intensity
	float intensity_step = 1.0 / u_Steps;
	float lightIntensity = light.intensity;
	for (int i = 0; i < u_Steps; ++i)
	{
		if (lightIntensity < intensity_step * (i + 1))
		{
			lightIntensity = intensity_step * i;
			break;
		}
	}

	if(u_HasNormalMap == 1)
		return CalculateLightResult(light.color, /*v_TBN * */normalize(light.dir), normal, viewDir, light.LightCastingShadows) * lightIntensity;
	else
		return CalculateLightResult(light.color, light.dir, normal, viewDir, light.LightCastingShadows) * lightIntensity;
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

	float intensity_step = 1.0 / u_Steps;
	float lightIntensity = light.intensity * lightAttenuation;
	for (int i = 0; i < u_Steps; ++i)
	{
		if (lightIntensity < intensity_step * (i + 1))
		{
			lightIntensity = intensity_step * i;
			break;
		}
	}

	//Result
	return CalculateLightResult(light.color, direction, normal, viewDir, false) * lightIntensity;
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

	float intensity = clamp((theta - light.InOutCutoff.y) / epsilon, 0.0, 1.0) * light.intensity;
	float intensity_step = 1.0 / u_Steps;
	float lightIntensity = intensity * lightAttenuation;
	for (int i = 0; i < u_Steps; ++i)
	{
		if (lightIntensity < intensity_step * (i + 1))
		{
			lightIntensity = intensity_step * i;
			break;
		}
	}

	//Result
	return CalculateLightResult(light.color, direction, normal, viewDir, false) * lightIntensity;
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
			alpha = v_Color.a;
		else
			alpha = texture(u_AlbedoTexture, v_TexCoord).a * v_Color.a;
	}

	if(alpha < 0.004)
		discard;

	//Normal Mapping Calculations
	vec3 normalVec = normalize(v_Normal);	
	if(u_DrawNormalMapping == 1)
	{
		out_color = vec4(normalVec, 1.0);
		return;
	}
	
	vec3 viewDirection = normalize(v_CamPos - v_FragPos);
	if(u_HasNormalMap == 1)
	{
		normalVec = texture(u_NormalTexture, v_TexCoord).rgb;
		normalVec = normalize(normalVec * 2.0 - 1.0);
		normalVec = normalize(v_TBN * normalVec);
		//viewDirection = v_TBN * normalize(v_CamPos - v_FragPos);
	}

	//Light Calculations
	int lights_iterator = (u_LightsNumber > MAX_SHADER_LIGHTS ? MAX_SHADER_LIGHTS : u_LightsNumber);
	vec3 colorResult = vec3(0.0);	
	for(int i = 0; i < lights_iterator; ++i)
	{
		//If we don't have to draw normal map debug
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
				colorResult += v_TBN * normalize(u_BkLights[i].dir);
			else
				colorResult += v_TBN * normalize(u_BkLights[i].pos);
		}
	}
	

	if(u_DrawNormalMapping_Lit == 0 && u_DrawNormalMapping_Lit_Adv == 0)
	{
		vec3 finalColor = v_Color.rgb;
		if(u_SceneColorAffected)
			finalColor *= u_AmbientColor.rgb;

		//Resulting Color
		if(u_UseTextures == 0 || (u_HasTransparencies == 0 && u_UseTextures == 1 && texture(u_AlbedoTexture, v_TexCoord).a < 0.1))
			out_color = vec4(colorResult + finalColor, alpha);
		else if(u_UseTextures == 1)
			out_color = vec4(colorResult + finalColor * texture(u_AlbedoTexture, v_TexCoord).rgb, alpha);

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
