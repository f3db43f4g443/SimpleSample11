Texture2D Texture0;
SamplerState LinearSampler;
float h0, h1, k, g_deltaTime;
float2 texofs[4];
float2 g_invScreenResolution;

void PSUpdate( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	out float4 outColor : SV_Target )
{
	float4 texColor = Texture0.Sample( LinearSampler, tex );
	outColor = texColor;
	for( int i = 0; i < 4; i++ )
	{
		float4 texColor1 = Texture0.Sample( LinearSampler, tex + texofs[i] * g_invScreenResolution );
		float4 dColor = texColor1 - texColor;
		float dHeight = abs( dColor.w );
		float dHeight1 = clamp( dHeight - h0, 0, h1 );
		outColor += ( dHeight > 0 ? dHeight1 / dHeight : 0 ) * dColor;
	}
	float height1 = outColor.w - k * g_deltaTime;
	outColor = saturate( outColor * ( outColor.w > 0? height1 / outColor.w: 1 ) );
}

float3 baseColor;
void PSPrint_AlphaToOcclusion( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	in float4 instData : ExtraInstData0,
	out float4 outColor : SV_Target )
{
	float fHeightClip = instData.z;
	float4 texColor = Texture0.Sample( LinearSampler, tex );
	float fAlpha = texColor.w;
	fAlpha = fAlpha - fHeightClip;

	outColor = float4( texColor.xyz * baseColor, 1 );
	clip( fAlpha - 0.001 );
}