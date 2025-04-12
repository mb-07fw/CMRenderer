#if 1
cbuffer MVPMatrix : register(b0)
{
    matrix transform;
}

struct VSInput
{
    float3 pos : VertexPos3D;
    float2 texCoord : TexCoord;
};

struct VSOutput
{
    float2 texCoord : TexCoord;
    float4 pos : SV_Position;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    
    output.pos = float4(input.pos, 1.0f);
    
    output.pos = mul(output.pos, transform);
    
    output.texCoord = input.texCoord;
    
	return output;
}
#else


cbuffer MVPMatrix : register(b0)
{
    matrix transform;
}

struct VSInput
{
    float3 pos : VertexPos3D;
};

struct VSOutput
{
    float4 pos : SV_Position;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    
    output.pos = float4(input.pos, 1.0f);
    
    output.pos = mul(output.pos, transform);
    
    return output;
}

#endif