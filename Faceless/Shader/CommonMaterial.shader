Texture2D Texture0;
SamplerState LinearSampler;

void PSDefault( in float2 tex : TexCoord0,
	out float4 outColor[2] : SV_Target )
{
	outColor[0] = Texture0.Sample( LinearSampler, tex );
	outColor[1] = float4( 0, 0, 0, outColor[0].w );
	clip( outColor[0].w - 0.5 );
}

float g_occlusionValue;
void PSOcclusionDefault( in float2 tex : TexCoord0,
	out float4 outColor : SV_Target )
{
	outColor = float4( 1.0f, 1.0f, 1.0f, g_occlusionValue );
	clip( Texture0.Sample( LinearSampler, tex ).w - 0.5 );
}

void PSDefaultNoClip( in float2 tex : TexCoord0,
	out float4 outColor[2] : SV_Target )
{
	outColor[0] = Texture0.Sample( LinearSampler, tex );
	outColor[1] = float4( 0, 0, 0, outColor[0].w );
}

void PSOcclusionTextureAlphaNoClip( in float2 tex : TexCoord0,
	out float4 outColor : SV_Target )
{
	outColor = float4( 1.0f, 1.0f, 1.0f, Texture0.Sample( LinearSampler, tex ).x );
}

void PSWithColorData( in float2 tex : TexCoord0,
	in float4 instData : ExtraInstData0,
	out float4 outColor[2] : SV_Target )
{
	outColor[0] = Texture0.Sample( LinearSampler, tex ) * instData;
	outColor[1] = float4( 0, 0, 0, outColor[0].w );
}

void PSEmissionAlpha( in float2 tex : TexCoord0,
	in float4 instData : ExtraInstData0,
	out float4 outColor[2] : SV_Target )
{
	outColor[0] = 0;
	outColor[1] = Texture0.Sample( LinearSampler, tex ) * instData;
}