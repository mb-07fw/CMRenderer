struct VSInput
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float3 TexCoord : TEXCOORD0;
    float4 Inst_Transform_0 : INST_TRANSFORM0;
    float4 Inst_Transform_1 : INST_TRANSFORM1;
    float4 Inst_Transform_2 : INST_TRANSFORM2;
    float4 Inst_Transform_3 : INST_TRANSFORM3;
};

struct VSOutput
{
    float3 WorldPos : TEXCOORD0;
    float3 Normal : TEXCOORD1;
    float3 TexCoord : TEXCOORD2;
    float4 PositionH : SV_Position; // clip space
};

cbuffer CBCameraProj : register(b0)
{
    /* NOTE: THE ORDER OF THIS IS SUPERRRR IMPORTANT, MUST MATCH C++ SIDE OR COMPUTER WILL GO BOOM!!! */
    float4x4 View;
    float4x4 Projection;
};

VSOutput main( VSInput input )
{
    VSOutput output;
    
    float4x4 modelMatrix = float4x4(
        input.Inst_Transform_0,
        input.Inst_Transform_1,
        input.Inst_Transform_2,
        input.Inst_Transform_3
    );
    
    /* Transform position in world space. */
    float4 worldPos4 = mul(float4(input.Position, 1.0), modelMatrix);
    output.WorldPos = worldPos4.xyz;
    
    /* Transform normal to world space. */
    output.Normal = normalize(mul(float4(input.Normal, 0.0), modelMatrix).xyz);

    output.TexCoord = input.TexCoord;
    
    output.PositionH = mul(float4(output.WorldPos, 1.0), View);
    output.PositionH = mul(output.PositionH, Projection);

    return output;
}