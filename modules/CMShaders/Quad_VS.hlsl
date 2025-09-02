float4 main( float2 pos : Pos ) : SV_Position
{
    float4 pos4 = float4(pos, 1.0f, 1.0f);
    
    return pos4;
}