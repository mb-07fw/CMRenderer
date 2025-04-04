cbuffer Colors : register(b0)
{
    float4 colors[6]; // A array of normalied rgb values.
}

float4 main(uint colorIndex : SV_PrimitiveID) : SV_Target
{
    return colors[colorIndex / 2];
}