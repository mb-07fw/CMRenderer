struct VSInput
{
    float3 Position : POSITION;
    float3 Normal : NORMAL;
    float3 TexCoord : TEXCOORD0;
    float4 Inst_Transform_0 : INST_TRANSFORM0;
    float4 Inst_Transform_1 : INST_TRANSFORM1;
    float4 Inst_Transform_2 : INST_TRANSFORM2;
    float4 Inst_Transform_3 : INST_TRANSFORM3;
};

struct VSOutput
{
    float3 WorldPos : TEXCOORD0;
    float3 Normal : TEXCOORD1;
    float3 TexCoord : TEXCOORD2;
    float4 PositionH : SV_Position; // clip space
};

cbuffer CBCameraProj : register(b0)
{
    /* NOTE: THE ORDER OF THIS IS SUPERRRR IMPORTANT, MUST MATCH C++ SIDE OR COMPUTER WILL GO BOOM!!! */
    column_major float4x4 View;
    column_major float4x4 Projection;
};

VSOutput main( VSInput input )
{
    VSOutput output;
    
    column_major float4x4 modelMatrix = float4x4(
        input.Inst_Transform_0,
        input.Inst_Transform_1,
        input.Inst_Transform_2,
        input.Inst_Transform_3
    );
    
    /* Transform position in world space.
     * 
     * Note: Matrix * Vector is valid if the Matrix is column major. If the matrix is row major,
     *   the correct order is Vector * Matrix.
     */
    //float4 worldPos4 = mul(float4(input.Position, 1.0f), modelMatrix); // Row major
    float4 worldPos4 = mul(modelMatrix, float4(input.Position, 1.0f)); // Column major
    output.WorldPos = worldPos4.xyz;
    
    /* Transform normal in world space. */
    output.Normal = normalize(mul(float4(input.Normal, 0.0f), modelMatrix).xyz);

    output.TexCoord = input.TexCoord;
    
    output.PositionH = mul(float4(output.WorldPos, 1.0f), View);
    output.PositionH = mul(output.PositionH, Projection);

    return output;
}