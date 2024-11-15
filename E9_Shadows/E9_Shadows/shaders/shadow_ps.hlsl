
Texture2D shaderTexture : register(t0);
SamplerState diffuseSampler : register(s0);

Texture2D depthMap1Texture : register(t1);
Texture2D depthMap2Texture : register(t2);

SamplerState shadowSampler : register(s1);

// array it

#define MAX_LIGHTS 8

struct SpotLight
{
    // spotlight only
    float4 ambient;
    float4 diffuse;
    float4 specular;
    
    float3 position;
    float range;

    float3 direction;
    float spot;

    float3 attenuation;
    float materialSpecularPower;
    
    float4 color;
    float4 emissive;
	
};

struct DirectionalLight
{
    float4 diffuse;
    float4 ambient;
    
    float3 direction;
    float padding;

    float3 position;
    float padding2;
};

struct PointLight
{
    float4 diffuse;
    float4 ambient;
    
    float3 direction;
    float padding;

    float3 position;
    float padding2;
};

struct LightType
{
    bool lightEnabled;
    float3 padding;
    
    DirectionalLight directionalLight;
    PointLight pointLight;
    SpotLight spotlight;
    
    //float4 ambient;
    //float4 diffuse;
   // float3 direction;
};

cbuffer LightBuffer : register(b0)
{
    LightType lights[MAX_LIGHTS];
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
    
    float3 worldPosition : TEXCOORD1;
    float3 viewVector : TEXCOORD2;
    
    float4 lightViewPos[MAX_LIGHTS] : TEXCOORD3;
    
   
};

// Calculate lighting intensity based on direction and normal. Combine with light colour.
float4 calculateLighting(float3 lightDirection, float3 normal, float4 diffuse)
{
    float intensity = saturate(dot(normal, lightDirection));
    float4 colour = saturate(diffuse * intensity);
    return colour;
}

// Is the gemoetry in our shadow map
bool hasDepthData(float2 uv)
{
    if (uv.x < 0.f || uv.x > 1.f || uv.y < 0.f || uv.y > 1.f)
    {
        return false;
    }
    return true;
}

bool isInShadow(Texture2D sMap, float2 uv, float4 lightViewPosition, float bias)
{
    // Sample the shadow map (get depth of geometry)
    float depthValue = sMap.Sample(shadowSampler, uv).r;
	// Calculate the depth from the light.
    float lightDepthValue = lightViewPosition.z / lightViewPosition.w;
    lightDepthValue -= bias;

	// Compare the depth of the shadow map value and the depth of the light to determine whether to shadow or to light this pixel.
    if (lightDepthValue < depthValue)
    {
        return false;
    }
    return true;
}

float2 getProjectiveCoords(float4 lightViewPosition)
{
    // Calculate the projected texture coordinates.
    float2 projTex = lightViewPosition.xy / lightViewPosition.w;
    projTex *= float2(0.5, -0.5);
    projTex += float2(0.5f, 0.5f);
    return projTex;
}

float4 calculateDiffuse(float4 color, float3 lightVector, float3 normal)
{
    float NdotL = max(0, dot(normal, lightVector));
    return color * NdotL;
}

float4 calculateDiffuse(SpotLight light, float3 lightVector, float3 normal)
{
    float NdotL = max(0, dot(normal, lightVector));
    return light.color * NdotL;
}

// Spotlight tutorial: https://www.3dgep.com/texturing-lighting-directx-11/#Spotlight_Cone

float4 calculateSpecular(SpotLight light, float3 V, float3 L, float3 N)
{
    // Phong lighting.
    float3 R = normalize(reflect(-L, N));
    float RdotV = max(0, dot(R, V));

    // Blinn-Phong lighting
    float3 H = normalize(L + V);
    float NdotH = max(0, dot(N, H));

    return light.color * pow(RdotV, light.materialSpecularPower);
}

float calculateAttenuation(SpotLight light, float d)
{
    return 1.0f / (light.attenuation[0] + (light.attenuation[1] * d) + (light.attenuation[2] * d * d));
}

// Improved spotlight cone calculation
float calculateSpotCone(SpotLight light, float3 lightVector)
{
    float minCos = cos(light.spot); // The angle where light starts fading.
    float maxCos = (minCos + 1.0f) / 2.0f; // Smoother edge transition.
    float cosAngle = dot(light.direction.xyz, -lightVector);

    // Ensure smooth transition at the edges with a better-defined range
    return saturate((cosAngle - minCos) / (maxCos - minCos));
}

float4 main(InputType input) : SV_TARGET
{
    float shadowMapBias = 0.005f;
    float4 totalColour = float4(0.f, 0.f, 0.f, 1.f);
    float4 textureColour = shaderTexture.Sample(diffuseSampler, input.tex);
    Texture2D depthMapTextures[2] = { depthMap1Texture, depthMap2Texture };
    float4 globalAmbient = float4(0.2f, 0.2f, 0.2f, 1.0f);
    
    
    int lightsEnabled = 0;
    
    [unroll]
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        LightType thisLight = lights[i];
        if (thisLight.lightEnabled)
        {
            lightsEnabled++;
        }
    }
            
      
    
    [unroll]
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        LightType thisLight = lights[i];
        float4 colour = float4(0.0f, 0.0f, 0.0f, 1.0f);
        
        if (thisLight.lightEnabled)
        {
             // Calculate the projected texture coordinates.
            float2 pTexCoord = getProjectiveCoords(input.lightViewPos[i]);
            const bool isSpotlight = true;
            float4 lightAmbient;

            if (isSpotlight)
            {
                lightAmbient = thisLight.spotlight.ambient;
            }
            else
            {
                lightAmbient = thisLight.directionalLight.ambient;
            }

            // Shadow test. Is or isn't in shadow
            if (hasDepthData(pTexCoord))
            {
              

                // Has depth map data
                if (!isInShadow(depthMapTextures[i], pTexCoord, input.lightViewPos[i], shadowMapBias))
                {
                    // is NOT in shadow, therefore light
                    
                    if (isSpotlight)
                    {
                        SpotLight spotlight = thisLight.spotlight;
                        
                        float4 P = float4(input.worldPosition, 1);
                        float3 N = normalize(input.normal);
                        float3 V = normalize(input.viewVector - spotlight.position.xyz);

                        float3 L = (spotlight.position - P.xyz);
        
                        float distance = length(L);
                        L = L / distance;
        
                        float attenuation = calculateAttenuation(spotlight, distance);
                        float spotIntensity = calculateSpotCone(spotlight, L);
                
                        float4 calc_diffuse = calculateDiffuse(spotlight, L, N) * attenuation * spotIntensity;
                        float4 calc_specular = calculateSpecular(spotlight, V, L, N) * attenuation * spotIntensity;
        
                        calc_diffuse = saturate(calc_diffuse);
                        calc_specular = saturate(calc_specular);

                        float4 m_emissive = spotlight.emissive;
                        float4 m_ambient = spotlight.ambient * globalAmbient;
                        float4 m_diffuse = calc_diffuse;
                        float4 m_specular = calc_specular;
        
                        float4 finalColor = (globalAmbient + m_emissive + m_diffuse + m_specular) * textureColour; // (m_emissive + m_ambient + diffuse + specular) * textureColour;
                        lightAmbient = thisLight.spotlight.ambient;

                        colour = finalColor;
                        //return finalColor;
                    }
                    else
                    {
                        // Directional light
                        DirectionalLight directionalLight = thisLight.directionalLight;
                        colour = calculateLighting(-directionalLight.direction, input.normal, directionalLight.diffuse);

                    }
                    
                }
            }
            
            
            colour = saturate(colour + lightAmbient);

        }
      
        totalColour += colour / lightsEnabled;
	   
    }
    
    return saturate(totalColour) * textureColour;
}