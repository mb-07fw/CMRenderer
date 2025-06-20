cbuffer CameraTransform
{
    matrix CameraMVP;
};

float4 main( float2 pos : Pos2D ) : SV_Position
{
    float4 pos4 = float4(pos, 1.0f, 1.0f);
    
    return mul(pos4, CameraMVP);
}