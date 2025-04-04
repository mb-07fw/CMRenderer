float ScreenToClipX(uint x, uint width)
{
    return (2.0f * x) / (float)width - 1.0f;
}

float ScreenToClipY(uint y, uint height)
{
    return 1.0f - (2.0f * y) / (float)height;
}

float2 ScreenToClip2D(uint2 pos2D, uint width, uint height)
{
    return float2(ScreenToClipX(pos2D.x, width), ScreenToClipY(pos2D.y, height));
}