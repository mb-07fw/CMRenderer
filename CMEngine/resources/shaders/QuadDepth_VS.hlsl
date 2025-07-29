cbuffer CameraTransform
{
    matrix CameraMVP;
};

struct VSOutput
{
    float Depth : OutDepth;
    float4 Pos : SV_Position;
};

VSOutput main( float3 pos3 : Pos3D )
{
    VSOutput output;
    
    float4 pos4 = float4(pos3, 1.0f);
    
    output.Depth = pos3.z;
    output.Pos = mul(pos4, CameraMVP);
    
    return output;
}