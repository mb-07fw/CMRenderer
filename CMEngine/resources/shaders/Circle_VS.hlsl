cbuffer CameraTransform
{
    matrix CT_MVP;
};

struct VSInput
{
    float3 Pos : Pos3D;
    float Radius : InstanceRadius;
};

struct VSOutput
{
    float OutRadius : InstanceRadius;
    float2 OutUV : OutUV; /* Assumes that quad is [-0.5, 0.5] in NDC... */
    float4 OutPos : SV_Position;
};

VSOutput main( VSInput input )
{    
    VSOutput output;
    
    float4 pos4 = float4(input.Pos, 1.0f);
    
    output.OutRadius = input.Radius;
    
    /* Assumes that quad is [-0.5, 0.5] in NDC... */
    //output.OutUV = input.Pos.xy * input.Radius; // Should get multiplied by radius.
    output.OutUV = input.Pos.xy; 
    
    output.OutPos = mul(pos4, CT_MVP);
    
    return output;
}