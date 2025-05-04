float4 main(float depth : OutDepth) : SV_Target
{
    float depthFactor = ((depth + 1.5f) * 0.5f);
    
    return float4(1.0f * depthFactor, 0.0f, 0.0f, 1.0f);
}