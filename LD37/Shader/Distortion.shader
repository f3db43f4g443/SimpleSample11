Texture2D g_texTarget0;
Texture2D g_texTarget1;
SamplerState g_samplerPointClamp;
Texture2D Texture0;
Texture2D Texture1;
SamplerState Sampler;
SamplerState Sampler1;
float fMaxDistortion;
float2 g_invViewportSize;

void PSDistortionMaskColor( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	out float4 outColor[2] : SV_Target )
{
	float4 color = Texture0.Sample( Sampler, tex );
	float4 distortion = Texture1.Sample( Sampler1, tex );
	float2 texTarget = inPos.xy;
	texTarget += ( distortion.xw - distortion.zy ) * fMaxDistortion;
	texTarget *= g_invViewportSize;

	float4 dst0 = g_texTarget0.Sample( g_samplerPointClamp, texTarget );
	float4 dst1 = g_texTarget1.Sample( g_samplerPointClamp, texTarget );
	outColor[0].xyz = dst0.xyz * ( 1 - color.w );
	outColor[0].w = 1;
	outColor[1].xyz = color.xyz + dst1.xyz * ( 1 - color.w );
	outColor[1].w = 1;
}

void PSDistortionMaskOcc( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	out float4 outColor : SV_Target )
{
	float4 color = Texture0.Sample( Sampler, tex );
	float4 distortion = Texture1.Sample( Sampler1, tex );
	float2 texTarget = inPos.xy;
	texTarget += ( distortion.xw - distortion.zy ) * fMaxDistortion;
	texTarget *= g_invViewportSize;

	texTarget += ( distortion.xw - distortion.zy ) * g_invViewportSize * fMaxDistortion;

	float4 dst0 = g_texTarget0.Sample( g_samplerPointClamp, texTarget );
	outColor.xyz = min( dst0.xyz, color.xyz );
	outColor.w = dst0.w;
}