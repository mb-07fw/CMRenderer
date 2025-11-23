// Gltf_Texture_PS.hlsl
#include "Common.hlsl"

struct PSInput
{
    CM_POSITION WorldPos : TEXCOORD0;
    CM_NORMAL Normal : TEXCOORD1;
    CM_TEXCOORD TexCoord : TEXCOORD2;
};

cbuffer MaterialCB : register(b0)
{
    float4 BaseColor;
    float Metallic;
    float Roughness;
};

Texture2D g_Texture : register(t0);
SamplerState g_SamplerState : register(s0);

float4 main( PSInput input ) : SV_Target
{
    return BaseColor * g_Texture.Sample(g_SamplerState, input.TexCoord);
}