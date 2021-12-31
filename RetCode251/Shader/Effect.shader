Texture2D Texture0;
SamplerState LinearSampler;

void PSEmissionClearColorClip( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	out float4 outColor[2] : SV_Target )
{
	outColor[1] = Texture0.Sample( LinearSampler, tex );
	outColor[0] = float4( 0, 0, 0, outColor[1].w );
	clip( outColor[1].w - 0.5f );
}

void PSEmissionClearColorNoClip( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	out float4 outColor[2] : SV_Target )
{
	outColor[1] = Texture0.Sample( LinearSampler, tex );
	outColor[0] = float4( 0, 0, 0, outColor[1].w );
}

float2 g_bandDir;
void PSPhantomEffect( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	in float2 tex1 : TexCoord1,
	in float4 color : ExtraInstData0,
	out float4 outColor : SV_Target )
{
	float w = Texture0.Sample( LinearSampler, tex ).w;
	float k = dot( g_bandDir, tex1 );
	k = frac( k ) >= 0.5 ? 1 : 0;
	outColor = color * k;
	clip( w - 0.5 );
}