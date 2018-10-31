float g_totalTime;
float4 t[2];
float4 tx[2], ty[2];
float4 tb[2];
float4 baseColor[2];
Texture2D Texture0;
SamplerState Sampler;

float WaterFunc( in float2 tex, in float surface, in float fadeDist, in float2 dir )
{
	float f = dot( tb[0], abs( cos( tex.x * tx[0] + tex.y * ty[0] + t[0] * g_totalTime ) ) )
		+ dot( tb[1], abs( cos( tex.x * tx[1] + tex.y * ty[1] + t[1] * g_totalTime ) ) );

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

Texture2D g_texTarget0;
Texture2D g_texTarget1;
SamplerState g_samplerPointClamp;
float4 colortable[16];
float fColorTableSize;
float fMaxDistortion;
float2 g_invViewportSize;
float3 colorCoef;

float2 WaterFuncDistortion( in float2 tex, in float surface, in float fadeDist, in float2 dir )
{
	float4 k1 = sin( tex.x * tx[0] + tex.y * ty[0] + t[0] * g_totalTime );
	float4 k2 = sin( tex.x * tx[1] + tex.y * ty[1] + t[1] * g_totalTime );
	float kx = dot( tb[0], tx[0] * k1 ) + dot( tb[1], tx[1] * k2 );
	float ky = dot( tb[0], ty[0] * k1 ) + dot( tb[1], ty[1] * k2 );
	float fFade = saturate( ( dot( tex, dir ) - surface ) / fadeDist );
	return float2( kx, ky ) * ( 1 - fFade );
}

void PSWaterDistortion( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	in float2 tex1 : TexCoord1,
	in float4 outInstData : ExtraInstData0,
	out float4 outColor[2] : SV_Target )
{
	float f = WaterFunc( tex1, outInstData.x, outInstData.y, outInstData.zw );
	float2 distortion = WaterFuncDistortion( tex1, outInstData.x, outInstData.y, outInstData.zw );
	f = saturate( dot( float3( distortion.x * distortion.x, distortion.y * distortion.y, f ), colorCoef ) );
	int a = (int)floor( min( f, 0.9999 ) * fColorTableSize );
	float4 color = colortable[a];

	float2 texTarget = inPos.xy;
	texTarget += distortion * fMaxDistortion;
	texTarget *= g_invViewportSize;
	float4 dst0 = g_texTarget0.Sample( g_samplerPointClamp, texTarget );
	float4 dst1 = g_texTarget1.Sample( g_samplerPointClamp, texTarget );

	outColor[0].xyz = dst0.xyz * ( 1 - color.w );
	outColor[0].w = 1;
	outColor[1].xyz = color.xyz + dst1.xyz * ( 1 - color.w );
	outColor[1].w = 1;
}

void PSWaterDistortionOcc( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	in float2 tex1 : TexCoord1,
	in float4 outInstData : ExtraInstData0,
	out float4 outColor : SV_Target )
{
	float f = WaterFunc( tex1, outInstData.x, outInstData.y, outInstData.zw );
	float2 distortion = WaterFuncDistortion( tex1, outInstData.x, outInstData.y, outInstData.zw );
	f = saturate( dot( float3( distortion.x * distortion.x, distortion.y * distortion.y, f ), colorCoef ) );
	int a = (int)floor( min( f, 0.9999 ) * fColorTableSize );
	float4 color = colortable[a];

	float2 texTarget = inPos.xy;
	texTarget += distortion * fMaxDistortion;
	texTarget *= g_invViewportSize;
	float4 dst0 = g_texTarget0.Sample( g_samplerPointClamp, texTarget );

	outColor.xyz = min( dst0.xyz, color.xyz );
	outColor.w = dst0.w;
}


float WaterFunc1( in float2 tex, in float surface, in float fadeDist, in float2 dir )
{
	float4 tx0 = tx[0] * dir.x - ty[0] * dir.y;
	float4 ty0 = tx[0] * dir.y + ty[0] * dir.x;
	float4 tx1 = tx[1] * dir.x - ty[1] * dir.y;
	float4 ty1 = tx[1] * dir.y + ty[1] * dir.x;
	float f = dot( tb[0], abs( cos( tex.x * tx0 + tex.y * ty0 + t[0] * g_totalTime ) ) )
		+ dot( tb[1], abs( cos( tex.x * tx1 + tex.y * ty1 + t[1] * g_totalTime ) ) );

	return saturate( -surface / fadeDist + f );
}

float2 WaterFuncDistortion1( in float2 tex, in float surface, in float fadeDist, in float2 dir )
{
	float4 tx0 = tx[0] * dir.x - ty[0] * dir.y;
	float4 ty0 = tx[0] * dir.y + ty[0] * dir.x;
	float4 tx1 = tx[1] * dir.x - ty[1] * dir.y;
	float4 ty1 = tx[1] * dir.y + ty[1] * dir.x;

	float4 k1 = sin( tex.x * tx0 + tex.y * ty0 + t[0] * g_totalTime );
	float4 k2 = sin( tex.x * tx1 + tex.y * ty1 + t[1] * g_totalTime );
	float kx = dot( tb[0], tx0 * k1 ) + dot( tb[1], tx1 * k2 );
	float ky = dot( tb[0], ty0 * k1 ) + dot( tb[1], ty1 * k2 );
	float fFade = saturate( -surface / fadeDist );
	return float2( kx, ky ) * ( 1 - fFade );
}

void PSWaterDistortion1( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	in float2 tex1 : TexCoord1,
	in float4 outInstData : ExtraInstData0,
	out float4 outColor[2] : SV_Target )
{
	float f = WaterFunc1( tex1, outInstData.x, outInstData.y, outInstData.zw );
	float2 distortion = WaterFuncDistortion1( tex1, outInstData.x, outInstData.y, outInstData.zw );
	float2 distortion1 = float2( distortion.x * outInstData.z + distortion.y * outInstData.w, distortion.x * outInstData.w - distortion.y * outInstData.z );
	f = saturate( dot( float3( distortion1.x * distortion1.x, distortion1.y * distortion1.y, f ), colorCoef ) );
	int a = (int)floor( min( f, 0.9999 ) * fColorTableSize );
	float4 color = colortable[a];

	float2 texTarget = inPos.xy;
	texTarget += distortion * fMaxDistortion;
	texTarget *= g_invViewportSize;
	float4 dst0 = g_texTarget0.Sample( g_samplerPointClamp, texTarget );
	float4 dst1 = g_texTarget1.Sample( g_samplerPointClamp, texTarget );

	outColor[0].xyz = dst0.xyz * ( 1 - color.w );
	outColor[0].w = 1;
	outColor[1].xyz = color.xyz + dst1.xyz * ( 1 - color.w );
	outColor[1].w = 1;
}

void PSWaterDistortionOcc1( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	in float2 tex1 : TexCoord1,
	in float4 outInstData : ExtraInstData0,
	out float4 outColor : SV_Target )
{
	float f = WaterFunc1( tex1, outInstData.x, outInstData.y, outInstData.zw );
	float2 distortion = WaterFuncDistortion1( tex1, outInstData.x, outInstData.y, outInstData.zw );
	float2 distortion1 = float2( distortion.x * outInstData.z + distortion.y * outInstData.w, distortion.x * outInstData.w - distortion.y * outInstData.z );
	f = saturate( dot( float3( distortion1.x * distortion1.x, distortion1.y * distortion1.y, f ), colorCoef ) );
	int a = (int)floor( min( f, 0.9999 ) * fColorTableSize );
	float4 color = colortable[a];

	float2 texTarget = inPos.xy;
	texTarget += distortion * fMaxDistortion;
	texTarget *= g_invViewportSize;
	float4 dst0 = g_texTarget0.Sample( g_samplerPointClamp, texTarget );

	outColor.xyz = min( dst0.xyz, color.xyz );
	outColor.w = dst0.w;
}