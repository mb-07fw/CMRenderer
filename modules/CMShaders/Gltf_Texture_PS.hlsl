struct PSInput
{
    float3 WorldPos : TEXCOORD0;
    float3 Normal : TEXCOORD1;
    float3 TexCoord : TEXCOORD2;
};

cbuffer MaterialCB : register(b0)
{
    float4 BaseColor;
    float Metallic;
    float Roughness;
};

Texture2D inTexture : register(t0);
SamplerState samplerState : register(s0);

float4 main( PSInput input ) : SV_Target
{
    return BaseColor * inTexture.Sample(samplerState, (float2) input.TexCoord);
}
