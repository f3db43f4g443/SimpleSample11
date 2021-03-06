Texture2D Texture0;
SamplerState LinearSampler;

void PSColor( in float2 tex : TexCoord0,
	in float4 instData : ExtraInstData0,
	out float4 outColor[2] : SV_Target )
{
	float fHeightClip = instData.z;
	float4 texColor = Texture0.Sample( LinearSampler, tex );
	float fAlpha = texColor.w;
	fAlpha = fAlpha - fHeightClip;

	outColor[0] = texColor;
	outColor[1] = float4( 0, 0, 0, outColor[0].w );
	clip( fAlpha - 0.001 );
}

void PSOcclusion( in float2 tex : TexCoord0,
	in float4 instData : ExtraInstData0,
	out float4 outColor : SV_Target )
{
	float fBaseHeight = instData.x;
	float fHeightScale = instData.y;
	float fHeightClip = instData.z;
	float fAlpha = Texture0.Sample( LinearSampler, tex ).w;
	fAlpha = fAlpha - fHeightClip;
	outColor = 1;
	outColor.w = saturate( fAlpha * fHeightScale + fBaseHeight );
	clip( fAlpha - 0.001 );
}