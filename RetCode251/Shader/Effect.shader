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