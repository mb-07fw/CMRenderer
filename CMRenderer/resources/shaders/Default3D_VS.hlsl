cbuffer Transform : register(b0)
{
    matrix transform;
};

struct VSOutput
{
    float4 pos : SV_Position;
};

VSOutput main(float3 pos : Pos3D)
{
    VSOutput output;
    
    float4 pos4D = float4(pos, 1.0f);
    
    output.pos = mul(pos4D, transform);
    
    return output;
}