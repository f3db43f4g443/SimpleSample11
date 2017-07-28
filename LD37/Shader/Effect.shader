Texture2D Texture0;
SamplerState LinearSampler;
float4 Color0;
float4 Color1;

void PSTwoColorLerp( in float2 tex : TexCoord0,
	out float4 outColor : SV_Target )
{
	float4 texColor = Texture0.Sample( LinearSampler, tex );
	outColor.xyz = lerp( Color0.xyz, Color1.xyz, texColor.xyz );
	outColor.w = texColor.w;
}

void PSSingleColorEmissionClearColorClip( in float2 tex : TexCoord0,
	in float4 instData : ExtraInstData0,
	out float4 outColor[2] : SV_Target )
{
	outColor[1] = instData;
	outColor[0] = float4( 0, 0, 0, outColor[1].w );
	clip( outColor[1].w - 0.5f );
}

void PSSingleColorEmissionClearColorNoClip( in float2 tex : TexCoord0,
	in float4 instData : ExtraInstData0,
	out float4 outColor[2] : SV_Target )
{
	outColor[1] = instData;
	outColor[0] = float4( 0, 0, 0, outColor[1].w );
}

void PSEmissionClearColorClip( in float2 tex : TexCoord0,
	out float4 outColor[2] : SV_Target )
{
	outColor[1] = Texture0.Sample( LinearSampler, tex );
	outColor[0] = float4( 0, 0, 0, outColor[1].w );
	clip( outColor[1].w - 0.5f );
}

void PSEmissionClearColorNoClip( in float2 tex : TexCoord0,
	out float4 outColor[2] : SV_Target )
{
	outColor[1] = Texture0.Sample( LinearSampler, tex );
	outColor[0] = float4( 0, 0, 0, outColor[1].w );
}

void PSEmissionClearColorInstData( in float2 tex : TexCoord0,
	in float4 InstData : ExtraInstData0,
	out float4 outColor[2] : SV_Target )
{
	outColor[1] = Texture0.Sample( LinearSampler, tex ) * InstData;
	outColor[0] = float4( 0, 0, 0, outColor[1].w );
}

Texture2D Texture1;
float g_totalTime;
float fTimeScale;

void PSEmissionEffect( in float2 tex : TexCoord0,
	in float4 instData : ExtraInstData0,
	out float4 outColor[2] : SV_Target )
{
	float4 texColor = Texture0.Sample( LinearSampler, tex );
	float4 texColor1 = Texture1.Sample( LinearSampler, tex );
	float t = frac( g_totalTime * fTimeScale );
	t = abs( texColor1.x - t ) * 2;
	t = min( t, 2 - t );
	float alpha = lerp( texColor1.y, texColor1.z, t );
	outColor[1] = texColor * instData;
	outColor[1].w *= alpha;
	outColor[0] = float4( 0, 0, 0, outColor[1].w );
}

float2 tex1Size;
void PSEmissionEffect1( in float2 tex : TexCoord0,
	in float2 tex1 : TexCoord1,
	in float4 instData : ExtraInstData0,
	out float4 outColor[2] : SV_Target )
{
	float4 texColor = Texture0.Sample( LinearSampler, tex );
	float4 texColor1 = Texture1.Sample( LinearSampler, tex1 / tex1Size );
	float t = frac( g_totalTime * fTimeScale );
	t = abs( texColor1.x - t ) * 2;
	t = min( t, 2 - t );
	float alpha = lerp( texColor1.y, texColor1.z, t );
	outColor[1] = texColor * instData;
	outColor[1].w *= alpha;
	outColor[0] = float4( 0, 0, 0, outColor[1].w );
}

void PSParticleDissolveColor( in float2 tex : TexCoord0,
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
void PSParticleDissolveOcc( in float2 tex : TexCoord0,
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

void PSEmiRadialFade( in float2 tex : TexCoord0,
	in float4 instData : ExtraInstData0,
	in float4 instData1 : ExtraInstData1,
	out float4 outColor[2] : SV_Target )
{
	float4 texColor = Texture0.Sample( LinearSampler, tex ) * instData1;
	float2 fadeCenter = instData.xy;
	float fadeBegin = instData.z;
	float fadeEnd = instData.w;
	float fade = length( ( tex - fadeCenter ) );
	float alpha = saturate( ( fade - fadeEnd ) / ( fadeBegin - fadeEnd ) );
	texColor.w *= alpha;

	outColor[0] = float4( 0, 0, 0, texColor.w );
	outColor[1] = texColor;
}