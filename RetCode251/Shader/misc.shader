void PSCircle( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	in float4 instData : ExtraInstData0,
	out float4 outColor : SV_Target )
{
	float2 t = ( tex - float2( 0.5, 0.5 ) ) * 2;
	clip( 1 - length( t ) );
	outColor = instData;
}