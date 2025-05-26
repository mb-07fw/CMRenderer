cbuffer CameraTransform
{
    matrix CT_MVP;
};

struct VSInput
{
    float3 pos : VertexPos3D;
    float radius : InstanceRadius;
};

struct VSOutput
{
    float outRadius : InstanceRadius;
    float2 outUV : OutUV; /* Assumes that quad is [-0.5, 0.5] in NDC... */
    float4 outPos : SV_Position;
};

VSOutput main( VSInput input )
{    
    VSOutput output;
    
    float4 pos4 = float4(input.pos, 1.0f);
    
    output.outRadius = input.radius;
    
    /* Assumes that quad is [-0.5, 0.5] in NDC... */
    output.outUV = input.pos.xy;
    
    output.outPos = mul(pos4, CT_MVP);
    
    return output;
}