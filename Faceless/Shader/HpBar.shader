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
	float fPercent = fCur / fMax;
	float p0 = floor( fPercent * GridCount );
	float sat0 = fPercent * GridCount - p0;

	float fGrid = tex.x * GridCount;
	float sat = fGrid < p0 ? 1 : ( fGrid < p0 + 1 ? sat0 : 0 );
	
	int nGrid1 = (int)floor( fGrid );
	float l1 = ( ( 1 << nGrid1 ) & (int)fCur ) > 0 ? 1 : 0;
	int nGrid2 = (int)floor( GridCount - fGrid );
	float l2 = ( ( 1 << nGrid2 ) & (int)fMax ) > 0 ? 1 : 0;
	float l = tex.y < 0.5f ? l1 : l2;

	float4 full = lerp( ColorFull[0], ColorFull[1], l );
	float4 empty = lerp( ColorEmpty[0], ColorEmpty[1], l );
	float4 grey = lerp( ColorGrey[0], ColorGrey[1], l );
	outColor = lerp( grey, lerp( empty, full, fPercent ), sat );
}