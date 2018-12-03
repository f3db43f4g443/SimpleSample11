Texture2D Texture0;
SamplerState LinearSampler;

void PSColor( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
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

void PSOcclusion( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
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

void PSColorMulInstData( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	in float4 instData : ExtraInstData0,
	in float4 instData1 : ExtraInstData1,
	out float4 outColor[2] : SV_Target )
{
	float fHeightClip = instData.z;
	float4 texColor = Texture0.Sample( LinearSampler, tex ) * instData1;
	float fAlpha = texColor.w;
	fAlpha = fAlpha - fHeightClip;

	outColor[0] = texColor;
	outColor[1] = float4( 0, 0, 0, outColor[0].w );
	clip( fAlpha - 0.001 );
}

float4 vConstData;
void PSColorConstData( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	out float4 outColor[2] : SV_Target )
{
	float fHeightClip = vConstData.z;
	float4 texColor = Texture0.Sample( LinearSampler, tex );
	float fAlpha = texColor.w;
	fAlpha = fAlpha - fHeightClip;

	outColor[0] = texColor;
	outColor[1] = float4( 0, 0, 0, outColor[0].w );
	clip( fAlpha - 0.001 );
}

void PSOcclusionConstData( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	out float4 outColor : SV_Target )
{
	float fBaseHeight = vConstData.x;
	float fHeightScale = vConstData.y;
	float fHeightClip = vConstData.z;
	float fAlpha = Texture0.Sample( LinearSampler, tex ).w;
	fAlpha = fAlpha - fHeightClip;
	outColor = 1;
	outColor.w = saturate( fAlpha * fHeightScale + fBaseHeight );
	clip( fAlpha - 0.001 );
}