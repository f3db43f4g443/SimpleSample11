float g_totalTime;
float4 t[2];
float4 tx[2], ty[2];
float4 tb[2];
float4 baseColor[2];
Texture2D Texture0;
SamplerState Sampler;

float WaterFunc( in float2 tex, in float surface, in float fadeDist, in float2 dir )
{
	float f = dot( tb[0], cos( tex.x * tx[0] + tex.y * ty[0] + t[0] * g_totalTime ) * 0.5 + 0.5 );
		+ dot( tb[1], cos( tex.x * tx[1] + tex.y * ty[1] + t[1] * g_totalTime ) * 0.5 + 0.5 );

	return saturate( ( dot( tex, dir ) - surface ) / fadeDist + f );
}

void PSWater( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	in float2 tex1 : TexCoord1,
	in float4 outInstData : ExtraInstData0,
	out float4 outColor[2] : SV_Target )
{
	float f = WaterFunc( tex1, outInstData.x, outInstData.y, outInstData.zw );
	float4 texColor = Texture0.Sample( Sampler, tex );
	outColor[0] = texColor * lerp( baseColor[0], baseColor[1], f );
	outColor[1] = float4( 0, 0, 0, outColor[0].w );
}

void PSWaterOneColor( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	in float4 outInstData : ExtraInstData0,
	out float4 outColor : SV_Target )
{
	float f = WaterFunc( tex, outInstData.x, outInstData.y, outInstData.zw );
	outColor = lerp( baseColor[0], baseColor[1], f );
}