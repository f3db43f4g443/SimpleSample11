float4 ColorFull[2];
float4 ColorEmpty[2];
float4 ColorGrey[2];

void PSHpBarSmall( in float2 tex : TexCoord0,
	in float4 instData : ExtraInstData0,
	out float4 outColor : SV_Target )
{
	float fCur = instData.x;
	float fMax = instData.y;
	float GridCount = instData.z;
	float GridCount0 = instData.w;
	float fPercent = fCur / fMax;
	float p0 = floor( fPercent * GridCount );
	float sat0 = fPercent * GridCount - p0;

	float fGrid = tex.x * GridCount;
	float sat = fGrid < p0 ? 1 : ( fGrid < p0 + 1 ? sat0 : 0 );
	
	int nGrid1 = (int)floor( fGrid );
	float2 l1;
	l1.x = ( ( 1 << min( nGrid1, 31 ) ) & (int)fCur ) > 0 ? 1 : 0;
	l1.y = nGrid1 < GridCount0 ? 0 : 1;
	int nGrid2 = (int)floor( GridCount - fGrid );
	float2 l2;
	l2.x = ( ( 1 << min( nGrid2, 31 ) ) & (int)fMax ) > 0 ? 1 : 0;
	l2.y = nGrid2 < GridCount0 ? 0 : 1;
	float2 l = tex.y < 0.5f ? l1 : l2;
	l.y = l.x > 0 ? 1 : l.y;

	float4 full = lerp( ColorFull[0], ColorFull[1], l.x );
	float4 empty = lerp( ColorEmpty[0], ColorEmpty[1], l.x );
	float4 grey = lerp( ColorGrey[0], ColorGrey[1], l.x );
	outColor = lerp( grey, lerp( empty, full, fPercent ), sat ) * l.y;
}