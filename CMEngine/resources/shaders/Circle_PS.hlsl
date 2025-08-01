cbuffer FrameData
{
    float2 FD_ScreenResolution;
    float2 FD_Padding;
};

struct PSInput
{
    float Radius : InstanceRadius; /* TODO: Make radius in actual world units instead of being used as [0, 1]. */
    float2 UV : OutUV; /* Assumes that quad is [-0.5, 0.5] in NDC... */
};

float4 main( PSInput input ) : SV_Target
{
    /* Distance from circle center (0.0f, 0.0f) */
    float dist = 1.0f - length(input.UV);
    
    /* Remove gradient by making any distance > 0.0f equal to solid white. */
    //dist = step(0.0f, dist);
    
    /* Cool anti-aliasing with smoothstep... */
    dist = smoothstep(0.0f, 0.05f, dist);
    
    float4 outColor = float4(float3(dist, dist, dist), 1.0f);
    
    return float4(outColor);
}