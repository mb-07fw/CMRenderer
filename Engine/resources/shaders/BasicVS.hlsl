struct VSInput
{
    float3 Pos : POSITION;  
};

struct VSOutput
{
    float4 PosH : SV_Position;
};

VSOutput main( VSInput input )
{
    VSOutput output;
    
    output.PosH = float4(input.Pos, 1.0f);
    
    return output;
}