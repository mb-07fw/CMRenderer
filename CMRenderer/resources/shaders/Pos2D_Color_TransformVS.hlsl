struct VSOutput
{
    float4 pos : SV_Position;
};

cbuffer Transform
{
    matrix transform;
};

VSOutput main( float2 pos : POSITION )
{
    VSOutput output;
    
    output.pos = float4(pos, 0.0f, 1.0f);
    output.pos = mul(output.pos, transform);
    
	return output;
}