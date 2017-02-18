float g_totalTime;
float4 t[2];
float4 tx[2], ty[2];
float4 tb[2];
float4 baseColor[2];
float4 emiColor[2];

float WaterFunc( in float2 tex, in float surface, in float fadeDist )
{
	float f = dot( tb[0], cos( tex.x * tx[0] + tex.y * ty[0] + t[0] * g_totalTime ) * 0.5 + 0.5 );
		+ dot( tb[1], cos( tex.x * tx[1] + tex.y * ty[1] + t[1] * g_totalTime ) * 0.5 + 0.5 );

	return saturate( ( tex.y - surface ) / fadeDist + f );
}

void PSWater( in float2 tex : TexCoord0,
	in float4 outInstData : ExtraInstData0,
	out float4 outColor[2] : SV_Target )
{
	float f = WaterFunc( tex, outInstData.x, outInstData.y );
	outColor[0] = lerp( baseColor[0], baseColor[1], f );
	outColor[1] = lerp( emiColor[0], emiColor[1], f );
}

void PSWaterOneColor( in float2 tex : TexCoord0,
	in float4 outInstData : ExtraInstData0,
	out float4 outColor : SV_Target )
{
	float f = WaterFunc( tex, outInstData.x, outInstData.y );
	outColor = lerp( baseColor[0], baseColor[1], f );
}