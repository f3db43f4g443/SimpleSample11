Texture2D g_texTarget0;
SamplerState g_samplerPointClamp;
float2 g_invViewportSize;

void PSDistortion( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	in float4 inInstData[2] : ExtraInstData0,
	out float4 outColor : SV_Target )
{
	float3 mul = inInstData[0].xyz;
	float3 add = inInstData[1].xyz;
	float2 distortion = float2( inInstData[0].w, -inInstData[1].w );

	float2 texTarget = inPos.xy;
	texTarget += distortion;
	texTarget *= g_invViewportSize;

	float4 dst0 = g_texTarget0.Sample( g_samplerPointClamp, texTarget );
	outColor.xyz = add + dst0.xyz * mul;
	outColor.w = 1;
}

void PSDistortionAdd( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	in float4 inInstData[2] : ExtraInstData0,
	out float4 outColor : SV_Target )
{
	float2 texTarget = inPos.xy * g_invViewportSize;
	float4 color0 = g_texTarget0.Sample( g_samplerPointClamp, texTarget );

	float3 mul = inInstData[0].xyz;
	float3 add = inInstData[1].xyz;
	float2 distortion = float2( inInstData[0].w, -inInstData[1].w );

	texTarget = inPos.xy + distortion;
	texTarget *= g_invViewportSize;

	float4 dst0 = g_texTarget0.Sample( g_samplerPointClamp, texTarget );
	outColor.xyz = add + dst0.xyz * mul;
	outColor += color0;
	outColor.w = 1;
}

Texture2D Texture0;
SamplerState Sampler;
void PSDistortionMasked( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	in float4 inInstData[2] : ExtraInstData0,
	out float4 outColor : SV_Target )
{
	float3 mul = inInstData[0].xyz;
	float3 add = inInstData[1].xyz;
	float2 distortion = float2( inInstData[0].w, -inInstData[1].w );
	float4 mask = Texture0.Sample( Sampler, tex );
	distortion *= mask.zw;
	mul += ( 1 - mul ) * ( 1 - mask.x );
	add *= mask.y;

	float2 texTarget = inPos.xy;
	texTarget += distortion;
	texTarget *= g_invViewportSize;

	float4 dst0 = g_texTarget0.Sample( g_samplerPointClamp, texTarget );
	outColor.xyz = add + dst0.xyz * mul;
	outColor.w = 1;
}

void PSDistortionEffect( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	in float4 inInstData[2] : ExtraInstData0,
	out float4 outColor : SV_Target )
{
	float3 c0 = inInstData[0].xyz;
	float3 c1 = inInstData[1].xyz;
	float2 distortion = float2( inInstData[0].w, -inInstData[1].w );
	float4 color0 = Texture0.Sample( Sampler, tex );
	c0 *= color0.xyz;
	c1 = 1 - ( 1 - c1 ) * color0.w;

	distortion *= color0.w;
	float2 texTarget = inPos.xy;
	texTarget += distortion;
	texTarget *= g_invViewportSize;
	float4 dst0 = g_texTarget0.Sample( g_samplerPointClamp, texTarget );

	outColor.xyz = c0 + dst0.xyz * c1;
	outColor.w = 1;
}

float3 ColorRep;
void PSDistortionEffect1( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	in float4 inInstData[2] : ExtraInstData0,
	out float4 outColor : SV_Target )
{
	float3 c0 = inInstData[0].xyz;
	float3 c1 = inInstData[1].xyz;
	float2 distortion = float2( inInstData[0].w, -inInstData[1].w );
	float4 color0 = Texture0.Sample( Sampler, tex );
	c0 *= color0.xyz;
	float w = color0.w > 0.05f ? 1 : 0;
	c0.xyz += color0.w < 0.99f ? ColorRep * w : 0;
	c1 = 1 - ( 1 - c1 ) * w;

	distortion *= w;
	float2 texTarget = inPos.xy;
	texTarget += distortion;
	texTarget *= g_invViewportSize;
	float4 dst0 = g_texTarget0.Sample( g_samplerPointClamp, texTarget );

	outColor.xyz = c0 + dst0.xyz * c1;
	outColor.w = 1;
}

float g_totalTime;
void PSDistortionEffect2( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	in float4 inInstData[2] : ExtraInstData0,
	out float4 outColor : SV_Target )
{
	float3 mul = inInstData[0].xyz;
	float3 add = inInstData[1].xyz;
	float2 distortion = float2( inInstData[0].w, -inInstData[1].w );
	float t = abs( frac( tex.y / 16 + g_totalTime ) * 8 - 4 );
	float t0 = t * 0.25f;
	float3 mul1 = mul;
	mul = sqrt( mul );
	mul = step( t0, mul ) * mul * 0.3 + ( step( t0, mul1 ) ) * 0.7;

	float2 texTarget = inPos.xy;
	texTarget += distortion;
	texTarget *= g_invViewportSize;

	float4 dst0 = g_texTarget0.Sample( g_samplerPointClamp, texTarget );
	outColor.xyz = add + dst0.xyz * mul;
	outColor.w = 1;
}