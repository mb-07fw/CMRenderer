//cbuffer CameraTransform
//{
//    matrix CameraMVP;
//};

float4 main( float2 pos : WorldPos ) : SV_Position
{
    float4 pos4 = float4(pos, 1.0f, 1.0f);
    
    return pos4;
    
    //return mul(pos4, CameraMVP);
}