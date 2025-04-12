#if 1
Texture2D textureMap : register(t0);
SamplerState samplerState : register(s0);

float4 main(float2 texCoord : TexCoord) : SV_Target
{
    float4 sampledColor = textureMap.Sample(samplerState, texCoord);
    
    sampledColor.a = 1.0f;
    
    return sampledColor;
}
#else

float4 main() : SV_Target
{
    return float4(1.0f, 1.0f, 1.0f, 1.0f);
}
#endif