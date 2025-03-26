struct VSOutput
{
    float4 Pos : SV_Position;
};

VSOutput main( float2 pos : POSITION )
{
    VSOutput output;
    output.Pos = float4(pos, 0.0f, 1.0f);
    
    return output;
}