Texture2D g_texTarget0;
SamplerState g_samplerPointClamp;
float2 g_invViewportSize;

void PSOnProcessColorMat( in float4 inPos : SV_Position,
	in float2 tex : TexCoord0,
	in float4 instData : ExtraInstData0,
	in float4 instData1 : ExtraInstData1,
	in float4 instData2 : ExtraInstData2,
	out float4 outColor[2] : SV_Target )
{
	float2 texTarget = inPos.xy;
	texTarget *= g_invViewportSize;
	float4 texColor = g_texTarget0.Sample( g_samplerPointClamp, texTarget );
	texColor.xyz = float3( dot( texColor.xyz, instData.xyz ),
		dot( texColor.xyz, instData1.xyz ),
		dot( texColor.xyz, instData2.xyz ) ) + float3( instData.w, instData1.w, instData2.w );
	outColor[0].xyz = texColor.xyz;
	outColor[0].w = 1;
	outColor[1] = 0;
}