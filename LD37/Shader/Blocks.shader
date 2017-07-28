float4 InvDstSrcResolution;
float fDepth;

#include "Default2D.shh"

void VSScreen( in float2 pos : Position,
	in uint instID : SV_InstanceID,
	out float2 outTex : TexCoord0,
	out float4 outPos : SV_Position )
{
	float4 DstRect = g_insts[instID * 2];
	float4 SrcRect = g_insts[instID * 2 + 1];
	float2 vPos = ( pos * DstRect.zw + DstRect.xy ) * InvDstSrcResolution.xy;
	vPos = ( vPos * 2.0 - 1.0 ) * float2( 1.0, -1.0 );
	outPos = float4( vPos, fDepth, 1 );
	outTex = ( pos.xy * SrcRect.zw + SrcRect.xy ) * InvDstSrcResolution.zw;
}

void VSBlockRTLayer( in float2 tex : Position,
	in uint instID : SV_InstanceID,
	out float2 outTex : TexCoord0,
	out float2 outTex1 : TexCoord1,
	out float4 outPos : SV_Position )
{
	Default2D_Vertex( tex, instID * 3, outTex, outPos );

	float4 instData = g_insts[instID * 3 + 2];
	outTex1 = tex * instData.zw + instData.xy;
}

Texture2D Texture0;
Texture2D TextureMask;
SamplerState Sampler;

void PSBlockRTLayer( in float2 tex : TexCoord0,
	in float2 tex1 : TexCoord1,
	out float4 outColor[2] : SV_Target )
{
	float4 texColor = Texture0.Sample( Sampler, tex1 );
	float4 texMask = TextureMask.Sample( Sampler, tex );
	clip( texMask.w - 0.001 );
	outColor[0] = texColor;
	outColor[1] = float4( 0, 0, 0, 0 );
}