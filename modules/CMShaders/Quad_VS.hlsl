cbuffer MVPMatrix : register(b0)
{
    float4x4 mvp;
}

struct QuadVSOutput
{
    float depth : PXDepth;
    float4 pos : SV_Position;
};

QuadVSOutput main( float3 pos : Pos )
{
    QuadVSOutput output;

    float4 pos4 = float4(pos, 1.0f);
    
    output.pos = mul(pos4, mvp);
    output.depth = pos.z;
    
    return output;
}