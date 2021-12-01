Texture2D Texture0;
SamplerState PointSampler;

#define SAMPLE_SCALE 4
float4 filter[SAMPLE_SCALE * SAMPLE_SCALE * 4];
float4 colorShift;
float2 srcRes;

float4 colorCenter;
float4 colorEdge;
float edgeTexPow;

void PSPixelUpsample( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	out float4 outColor : SV_Target )
{
	float2 texscr = tex * srcRes;
	float2 texIndex = floor( frac( texscr ) * SAMPLE_SCALE );
	int nIndexBase = int( texIndex.x ) * 16 + int( texIndex.y ) * 4;

	float4 color = 0;
	for( int i = 0; i < 4; i++ )
	{
		float4 v = filter[nIndexBase + i];
		float2 tex1 = tex + v.zw;
		float4 c = v.x + v.y * colorShift;
		color += Texture0.Sample( PointSampler, tex1 ) * c;
	}

	float2 edge = tex;
	edge = abs( edge * 2 - 1 );
	edge = pow( edge, edgeTexPow );
	float t = edge.x + edge.y;
	float4 color1 = lerp( colorCenter, colorEdge, t );

	outColor = color * color1;
}