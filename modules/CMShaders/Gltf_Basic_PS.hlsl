struct PSInput
{
    float3 WorldPos : TEXCOORD0;
    float3 Normal : TEXCOORD1;
    float3 TexCoord : TEXCOORD2;
};

/* TODO: Implement texture support. */
cbuffer MaterialCB : register(b0)
{
    float4 BaseColor;
    float Metallic;
    float Roughness;
};

// cbuffer CameraCB : register(b1)
// {
//     float3 CameraPos;
// }
// 
// struct Light
// {
//     float3 Position;
//     float3 Color;
// };
// 
// cbuffer LightCB : register(b2)
// {
//     Light Lights[4];
//     uint NumLights;
// };

float4 main( PSInput input ) : SV_Target
{
    // float3 N = normalize(input.Normal);
    // float3 V = normalize(CameraPos - input.WorldPos);

    float3 color = BaseColor.rgb;

    /* Simple Lambert diffuse
    for (int i = 0; i < NumLights; i++)
    {
        float3 L = normalize(Lights[i].Position - input.WorldPos);
        float diff = max(dot(N, L), 0.0f);
        color += diff * Lights[i].Color;
    } */

    return float4(color, BaseColor.a);
}
