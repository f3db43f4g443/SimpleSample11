Texture2D Texture0;
SamplerState LinearSampler;

void PSActionEffect( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	in float4 instData : ExtraInstData0,
	out float4 outColor : SV_Target )
{
	outColor = instData * ( Texture0.Sample( LinearSampler, tex ).w > 0.05f ? 1 : 0 );
}

void PSTextureInstDataClip( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	in float4 instData : ExtraInstData0,
	out float4 outColor : SV_Target )
{
	outColor = instData * Texture0.Sample( LinearSampler, tex );
	clip( outColor.w - 0.5f );
}

void PSTextureColorRange( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	in float4 instData[2] : ExtraInstData0,
	out float4 outColor : SV_Target )
{
	outColor = Texture0.Sample( LinearSampler, tex );
	outColor = ( outColor - instData[0] ) / ( instData[1] - instData[0] );
}

float2 texSize;
float texSpeed;
float g_totalTime;
Texture2D Texture1;
SamplerState g_samplerPointWrap;
void PSChargeEffect( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	in float4 inInstData[2] : ExtraInstData0,
	out float4 outColor : SV_Target )
{
	float2 texTarget = inPos.xy;
	texTarget.y += g_totalTime * texSpeed;
	texTarget /= texSize;

	float4 color0 = Texture0.Sample( LinearSampler, tex );
	float4 color1 = Texture1.Sample( g_samplerPointWrap, texTarget );
	color1 = inInstData[0] * color1 + inInstData[1];
	float4 color2 = color0;
	color2.xyz += ( color1.xyz - color2.xyz ) * color1.w;
	color0.xyz = color0.w < 0.99f ? color2.xyz : color0.xyz;

	outColor = color0;
	outColor.w = 1;
	clip( color0.w - 0.5f );
}