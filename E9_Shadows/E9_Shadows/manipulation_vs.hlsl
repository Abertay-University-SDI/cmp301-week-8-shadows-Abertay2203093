// Light vertex shader
// Standard issue vertex shader, apply matrices, pass info to pixel shader
cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer TimeBuffer : register(b1)
{
    float time;
    float amplitude;
    float frequency;
    float speed;
    
    float4 colourTexture;
    
    int isWaveManipulation;
    int isHeightMapManipulation;
    float2 padding;
}

struct InputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

struct OutputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float4 colourTex : TEXCOORD1;
};

Texture2D colorMap : register(t1);
SamplerState colorSampler : register(s1);

float GetHeight(float2 uv, float scale)
{
    float4 colors = colorMap.SampleLevel(colorSampler, uv, 0);
    float height = colors.r * scale;
    return height;
}

OutputType main(InputType input)
{
    OutputType output;
				
    if (isWaveManipulation == 1)
    {
		// offset position based on sine wave
        float spd = time * speed;
	
        float sin_wave = sin(frequency * (input.position.x + spd)) * amplitude;
        float cos_wave = cos(frequency * (input.position.z + spd)) * amplitude;

        float x_normal = -cos(frequency * (input.position.x + spd));
        float z_normal = -sin(frequency * (input.position.z + spd));

        input.position.y = sin_wave + cos_wave;
		
		// modify normal
        input.normal = float3(x_normal, 1, z_normal);
    }
	
    if (isHeightMapManipulation == 1)
    {
		// https://stackoverflow.com/questions/27262817/directx-hlsl-texture-based-height-map-shader-5-0

        input.position.y = GetHeight(input.tex, speed);
        //input.normal = CalculateNormals(input.tex);
				
    }
	
	


	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

	// Store the texture coordinates for the pixel shader.
    output.tex = input.tex;
    output.colourTex = colourTexture;

	// Calculate the normal vector against the world matrix only and normalise.
    output.normal = mul(input.normal, (float3x3) worldMatrix);
    output.normal = normalize(output.normal);

    return output;
}