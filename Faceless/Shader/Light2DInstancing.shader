struct SInstData
{
	float4 SrcRect;
	float4 DstRect;
	float4 LightData;
};

struct SInstData1
{
	float4 DstRect;
	float4 SrcRect;
	float4 SrcRect1;
	float4 LightData;
	float4 LightData1;
	float2 RadialBlurCenter;
};

cbuffer InstBuffer
{
	SInstData g_insts[1365];
};

cbuffer InstBuffer1
{
	SInstData1 g_insts1[682];
};
float4 InvDstSrcResolution;

void VSRadialBlur( in float2 pos : Position,
	in uint instID : SV_InstanceID,
	out float2 outTex : TexCoord0,
	out float4 outLightData : TexCoord1,
	out float4 outPos : SV_Position )
{
	SInstData instData = g_insts[instID];
	float4 DstRect = instData.DstRect;
	float4 SrcRect = instData.SrcRect;

	float2 vPos = ( pos * DstRect.zw + DstRect.xy ) * InvDstSrcResolution.xy;
	vPos = ( vPos * 2.0 - 1.0 ) * float2( 1.0, -1.0 );
	outPos = float4( vPos, 0, 1 );
	outTex = ( pos.xy * SrcRect.zw + SrcRect.xy ) * InvDstSrcResolution.zw;
	outLightData = instData.LightData;
}

void VSRadialBlur1( in float2 pos : Position,
	in uint instID : SV_InstanceID,
	out float4 outTex : TexCoord0,
	out float4 outLightData : TexCoord1,
	out float4 outLightData1 : TexCoord2,
	out float2 outRadialBlurCenter : TexCoord3,
	out float4 outPos : SV_Position )
{
	SInstData1 instData = g_insts1[instID];
	float4 DstRect = instData.DstRect;
	float4 SrcRect = instData.SrcRect;
	float4 SrcRect1 = instData.SrcRect1;

	float2 vPos = ( pos * DstRect.zw + DstRect.xy ) * InvDstSrcResolution.xy;
	vPos = ( vPos * 2.0 - 1.0 ) * float2( 1.0, -1.0 );
	outPos = float4( vPos, 0, 1 );
	outTex.xy = ( pos.xy * SrcRect.zw + SrcRect.xy ) * InvDstSrcResolution.zw;
	outTex.zw = ( pos.xy * SrcRect1.zw + SrcRect1.xy ) * InvDstSrcResolution.zw;
	outLightData = instData.LightData;
	outLightData1 = instData.LightData1;
	outRadialBlurCenter = instData.RadialBlurCenter;
}