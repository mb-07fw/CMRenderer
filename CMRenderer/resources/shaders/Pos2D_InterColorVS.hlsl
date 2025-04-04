//#include "Utility.hlsl"

float ScreenToClipX(uint x, uint width)
{
    return (2.0f * (float)x) / width - 1.0f;
}

float ScreenToClipY(uint y, uint height)
{
    return 1.0f - (2.0f * (float)y) / height;
}

float2 ScreenToClip2D(uint2 pos2D, uint width, uint height)
{
    return float2(ScreenToClipX(pos2D.x, width), ScreenToClipY(pos2D.y, height));
}

cbuffer ViewportData
{
    uint ViewportWidth;
    uint ViewportHeight;
};

struct VSOutput
{
    float4 color : InterColor;
    float4 pos : SV_Position;
};

VSOutput main(float4 color : InterColor, uint2 pos : Pos2D)
{
    VSOutput output;
    
    float2 normPos = ScreenToClip2D(pos, ViewportWidth, ViewportHeight);

    output.color = color;
    output.pos = float4(normPos, 0.0f, 1.0f);
    
    return output;
}