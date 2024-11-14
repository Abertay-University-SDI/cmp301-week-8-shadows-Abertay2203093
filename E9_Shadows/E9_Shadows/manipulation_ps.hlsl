// Light pixel shader
// Calculate diffuse lighting for a single directional light (also texturing)

Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

Texture2D texture1 : register(t1);
SamplerState sampler1 : register(s1);

cbuffer LightBuffer : register(b0)
{
    float4 diffuseColour;
    float3 lightDirection;
    float padding;
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

// Calculate lighting intensity based on direction and normal. Combine with light colour.
float4 calculateLighting(float3 lightDirection, float3 normal, float4 diffuse)
{
    float intensity = saturate(dot(normal, lightDirection));
    float4 colour = saturate(diffuse * intensity);
    return colour;
}

float4 main(InputType input) : SV_TARGET
{
    float4 heightMapColour;
    float4 lightColour;
    float4 textureColour;

	// Sample the texture. Calculate light intensity and colour, return light*texture for final pixel colour.
    heightMapColour = texture0.Sample(sampler0, input.tex);
    textureColour = texture1.Sample(sampler1, input.tex);
    
    lightColour = calculateLighting(-lightDirection, input.normal, diffuseColour);
	
    return lightColour * heightMapColour * textureColour;

}



