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