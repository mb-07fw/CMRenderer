cbuffer CameraTransform
{
    matrix CameraMVP;
};

float4 main(float2 uv : Screen_Pos2D) : SV_Position
{
    float4 uv4 = float4(uv, 1.0f, 1.0f);
    
    return mul(uv4, CameraMVP);
}