float4 main( float depth : PXDepth) : SV_Target
{
	return float4(depth, 0.0f, 0.0f, 1.0f);
}