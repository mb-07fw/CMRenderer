struct VSInput
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
};

struct VSOutput
{
    float3 WorldPos : TEXCOORD0;
    float3 Normal : TEXCOORD1;
    float2 TexCoord : TEXCOORD2;
    float4 PositionH : SV_Position; // clip space
};

cbuffer TransformCB : register(b0)
{
    float4x4 Model;
    float4x4 View;
    float4x4 Projection;
};

VSOutput main( VSInput input )
{
    VSOutput output;
    
    /* Transform position in world space. */
    float4 worldPos4 = mul(float4(input.Position, 1.0), Model);
    output.WorldPos = worldPos4.xyz;
    
    /* Transform normal to world space. */
    output.Normal = normalize(mul(float4(input.Normal, 0.0), Model).xyz);

    output.TexCoord = input.TexCoord;
    
    output.PositionH = mul(float4(output.WorldPos, 1.0), View);
    output.PositionH = mul(output.PositionH, Projection);

    return output;
}