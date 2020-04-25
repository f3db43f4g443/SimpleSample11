Texture2D Texture0;
SamplerState LinearSampler;

void PSActionEffect( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	in float4 instData : ExtraInstData0,
	out float4 outColor : SV_Target )
{
	outColor = instData * Texture0.Sample( LinearSampler, tex ).w;
}

void PSTextureInstDataClip( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	in float4 instData : ExtraInstData0,
	out float4 outColor : SV_Target )
{
	outColor = instData * Texture0.Sample( LinearSampler, tex );
	clip( outColor.w - 0.5f );
}


void PSParticleDissolveColor( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	in float4 instData : ExtraInstData0,
	out float4 outColor[2] : SV_Target )
{
	float fHeightClip = 1 - instData.w;
	float4 texColor = Texture0.Sample( LinearSampler, tex );
	float fAlpha = texColor.w;
	fAlpha = fAlpha - fHeightClip;

	outColor[0] = texColor;
	outColor[1] = float4( 0, 0, 0, outColor[0].w );
	clip( fAlpha - 0.001 );
}

float fMinHeight;
float fMaxHeight;
void PSParticleDissolveOcc( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	in float4 instData : ExtraInstData0,
	out float4 outColor : SV_Target )
{
	float fHeightClip = 1 - instData.w;
	float4 texColor = Texture0.Sample( LinearSampler, tex );
	float fAlpha = texColor.w;
	fAlpha = fAlpha - fHeightClip;

	outColor = float4( 1, 1, 1, lerp( fMinHeight, fMaxHeight, texColor.w ) );
	clip( fAlpha - 0.001 );
}